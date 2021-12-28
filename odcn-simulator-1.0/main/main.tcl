# main.tcl #

# Simulation script of OBS in Datacenter Network devlopped by Salek Riadi#

# Test network case: one Control Node; M Core Nodes; N Pod Nodes; 10 GB/s channels; DW Data Wavelengths; CW Control Wavelengths;
# Read cmd line parameters : ns main.tcl N M DW CW BW PD PT BL L
# Exemple: ns main.tcl 200 20 8 8 10000000000 0.0000005 0.000001 1000000 0.5


# N: number of pod switches
set N [lindex $argv 0]

# M: number of core switches
set M [lindex $argv 1]

# DW: number of data wavelengths per single fiber
set DW [lindex $argv 2] 

# CW: number of control wavelengths per single fiber
set CW [lindex $argv 3] 

# BW: bandwidth/channel (10000000000)
set BW [lindex $argv 4] 

# PD: propagation delay
set PD [lindex $argv 5] 

# PT: processing time (0.000001)
set PT [lindex $argv 6]

# BL: burst length/size (1000000)
set BL [lindex $argv 7]

# L: Load
set L [lindex $argv 8]

# Specific simulation libraries of OBS 												
source ../tcl/lib/ns-obs-lib.tcl
source ../tcl/lib/ns-obs-defaults.tcl
source ../tcl/lib/ns-optic-link.tcl

# Create a simulator object
set ns [new Simulator]

# Type of classifiers for simulation of edge, core and conntrol nodes
Classifier/BaseClassifier/EdgeClassifier set type_ 0
Classifier/BaseClassifier/CoreClassifier set type_ 1
Classifier/BaseClassifier/ControllerClassifier set type_ 2

# BHP (control packet) processing time (1 microsecond = 0.000001 second)
Classifier/BaseClassifier/CoreClassifier set bhpProcTime $PT
Classifier/BaseClassifier/EdgeClassifier set bhpProcTime $PT
Classifier/BaseClassifier/ControllerClassifier set bhpProcTime $PT

# Number of pod and core nodes
Classifier/BaseClassifier/ControllerClassifier set numberofpodnodes $N
Classifier/BaseClassifier/ControllerClassifier set numberofcorenodes $M

# Number of data wavelengths
Classifier/BaseClassifier/ControllerClassifier set numberofwavelengths $DW

# Bandwidth per channel (10Gb/s = 10000000000bit/s)
Classifier/BaseClassifier/CoreClassifier set bandwidthperchannel $BW
Classifier/BaseClassifier/ControllerClassifier set bandwidthperchannel $BW

# Speed of light in fiber optic is 200000000m/s
# Propagation Delay of fibers between nodes (for 100m of fiber optic, Propagation Delay will be 0.0000005s = 100/200000000)
Classifier/BaseClassifier/ControllerClassifier set propagationdelay $PD

# Global FDL option
# 	if option == 0 don't use FDLs
#	if option == 1 max #FDLs per node
#	if option == 2 max #FDLs per path
Classifier/BaseClassifier set option 0

# Option for electronic buffering at edge nodes
# 	if ebufoption == 0:  drop BHPs and bursts if burst can't be scheduled at offset time (but buffer burst that can be scheduled until they are sent)
#   if ebufoption == 1:  for bursts that can't be scheduled at offset time, schedule at earliest opportunity; buffer them until then.
Classifier/BaseClassifier set ebufoption 1

# This is a fixed delay line present at the ingress of every node
OBSFiberDelayLink set FDLdelay 0.0

# Delay in seconds: between two adjacent nodes
set delay $PD

# Number of control wavelengths per fiber optic
set ncc $CW

# Number of data wavelengths per fiber optic
set ndc $DW

# Create datacenter topology
Simulator instproc create_topology { } {
    $self instvar Node_
    global E C 
    global N M
    global BW ncc ndc delay
	global PT

	# set up the controller node
    set Controller [$self create-controller-node $N $M $ndc $BW $delay $PT]
    set nid [$Controller id]
    set string1 "Controller node id:     $nid"
    puts $string1
	
	set i 0
    # set up the core nodes
    while { $i < $M } {
		set C($i) [$self create-core-node $M]
        set nid [$C($i) id]
        set string1 "C($i) node id:     $nid"
        puts $string1
		incr i
    }

	set i 0
    # set up the edge nodes
    while { $i < $N } {
		set E($i) [$self create-edge-node $M $N]
        set nid [$E($i) id]
        set string1 "E($i) node id:     $nid"
        puts $string1
		incr i
    }
    
	# set up the optical fibers between pod nodes and core nodes 
	for {set i 0} {$i < $N} {incr i} {
		for {set j 0} {$j < $M} {incr j} {
			$self createDuplexFiberLink $E($i) $C($j) $BW "[expr 1000. * $delay]ms" 0 $ndc [expr 0 + $ndc]
		}
	}
	
	# set up the optical fibers between controller node and pod nodes
	for {set i 0} {$i < $N} {incr i} {
		$self createDuplexFiberLink $Controller $E($i) $BW "[expr 1000. * $delay]ms" $ncc 0 [expr $ncc + 0]
	}
	
	# set up the optical fibers between controller node and core nodes
	for {set j 0} {$j < $M} {incr j} {
		$self createDuplexFiberLink $Controller $C($j) $BW "[expr 1000. * $delay]ms" $ncc 0 [expr $ncc + 0]
	}

    $self build-routing-table
}

$ns create_topology

# Create a traffic-stream over a UDP agent
Simulator instproc  create_connection { udp null src dest } {
     upvar 1 $udp udpr
     upvar 1 $null nullr
     upvar 1 $src srcr
     upvar 1 $dest destr
     set udpr [ new Agent/UDP]
     $self attach-agent $srcr $udpr
     set nullr [ new Agent/Null ]
     $self attach-agent $destr $nullr
     $self connect $udpr $nullr
     puts "traffic stream between $src = $srcr and $dest = $destr created"
}

# number of class of service
set S 1

# Add traffic stream between every pair of edge nodes in both directions
for {set i 0} {$i < $N} {incr i} {
	for {set j 0} {$j < $N} {incr j} {
		if { $j != $i } {
			for {set k 0} {$k < $S} {incr k} {
				$ns  create_connection  udp($i,$j,$k) null($i,$j,$k) E($i) E($j)
				$udp($i,$j,$k) set prio_ $k
			}
		}
	}
}

# getdurationofburst procedure
proc getdurationofburst { burstsize_ bwpc_ } {
	# the value of bwpc_ is in bits per second (bit/s)
	# the value of burstsize_ is in bytes and 1 byte = 8 bits
	# therefore, duration of burst is given by
	return [expr 8. * $burstsize_ / ( 1. *  $bwpc_)]
}	

# getsizeofburst procedure
proc getsizeofburst { burstduration_ bwpc_ } {
	# the value of burstduration_ is in seconds
	# the value of size of burst in bytes is given by
	return [expr ( 1. *  $bwpc_ * $burstduration_ )/8.]
}	

# The burst arrival from pod node is assumed Poisson process with rate λ bursts per second 
# and the duration of burst is exponentially distributed with a mean of 1/µ seconds.
# I can set the load (L) generated by each burst assembler by adjusting the rate λ: L = λ/(DW.M.µ)

# mu µ is the inverse of burst duration (seconds)
set mu [expr 1./[getdurationofburst $BL $BW]]


# Consequently, the rate of burst arrival lambda λ (bursts per second) is given by λ = DW.M.µ.L
set lambda [expr $M*$DW*$mu*$L]

# number of generated bursts
set num_of_bursts 0

# number of generated bursts for each class of service
for {set i 0} {$i < $S} {incr i} {
	set num_of_burstsperclass($i) 0
}

# Application layer
# Each pod node (source_) is associated with a burst generator procedure 
# class_ represents the class of service
proc burstgenerator { source_ class_ } {
	global ns udp lambda mu BW N num_of_bursts num_of_burstsperclass
	if { $num_of_bursts == 1001000 } {
		# Simulation is complete, when number of generated bursts reaches a certain number (the maximum limit) 
		# finish procedure is called to collect statistics
		finish
	}
	
	# incrementation of number of generated bursts 
	incr num_of_bursts
	incr num_of_burstsperclass($class_)

	# destination pod node is randomly selected using uniform distribution
	set destination_ [expr int(rand() * $N)]
	while {$destination_ == $source_} {
		set destination_ [expr int(rand() * $N)]
	}
	
	# on_time is the duration of burst which is exponentially distributed with a mean of burstduration (1/µ)
	set on_time [expr (-log(-rand()+1))/$mu];

	set size [getsizeofburst $on_time $BW]	
	BurstManager maxburstsize $size

	# source pod node send new burst destination pod node
	$udp($source_,$destination_,$class_) send $size
	
	# idle_time represents the inter-arrival of bursts is exponentially distributed with a mean of 1/lambda (1/λ)
	set idle_time [expr (-log(-rand()+1)/$lambda)];
	set now [$ns now]

	# burstgenerator procedure is called recursively after idle_time to send next burst
	$ns at [expr $now+$idle_time] "burstgenerator $source_ $class_"
}

# Create a statistics collector
StatCollector set debug_ 0
set sc [new StatCollector]

# Add new types of statistics
set stattype "BURSTQDLY"
$sc add-stat-type $stattype 0
set stattype "BURSTWAITTIME"
$sc add-stat-type $stattype 0
set stattype "BURSTDURATION"
$sc add-stat-type $stattype 0
for {set i 0} {$i < $M} {incr i} {
	set stattype "BURSTRCVC$i"
	$sc add-stat-type $stattype 0
	set stattype "BYTESRCVC$i"
	$sc add-stat-type $stattype 0
	set stattype "BURSTDLYC$i"
	$sc add-stat-type $stattype 0
	set stattype "BURSTQDLYC$i"
	$sc add-stat-type $stattype 0
	set stattype "BURSTWAITTIMEC$i"
	$sc add-stat-type $stattype 0
}

# savestatistics procedure
proc savestatistics {stattype value} {
	set path "statistics/$stattype.out"
	set statfile [open $path a]
	puts $statfile $value
	close $statfile
}

# real launch time of simulation
set real_launch_time [clock microseconds]

# finish procedure
proc finish {} {
    global ns sc N M DW CW BW PD PT BL L lambda mu delay num_of_bursts num_of_burstsperclass real_launch_time
	$sc display-sim-list
	
	set br [$sc getValue "BURSTRCV"]
	set bl [$sc getValue "BURSTDROP"]
	set bs [$sc getValue "BURSTSND"]	
	set tp [$sc getValue "BYTESRCV"]
	set bd [$sc getValue "BURSTDLY"]
	set bqd [$sc getValue "BURSTQDLY"]
	set bwt [$sc getValue "BURSTWAITTIME"]
	set burstduration [$sc getValue "BURSTDURATION"]
	set cps [$sc getValue "BHPSND"]
	set cl [$sc getValue "BHPDROP"]

	for {set i 0} {$i < $M} {incr i} {
		set brc($i) [$sc getValue "BURSTRCVC$i"]
		set tpc($i) [$sc getValue "BYTESRCVC$i"]
		set bdc($i) [$sc getValue "BURSTDLYC$i"]
		set bqdc($i) [$sc getValue "BURSTQDLYC$i"]
		set bwtc($i) [$sc getValue "BURSTWAITTIMEC$i"]
		if {$brc($i) != 0} {
			set BD($i) [expr $bdc($i) / $brc($i)]
			set BQD($i) [expr $bqdc($i) / $brc($i)]
			set BWT($i) [expr $bwtc($i) / $brc($i)]
		} else {
			set BD($i) 0
			set BQD($i) 0
			set BWT($i) 0
		}
	}
	
	set DS [$ns now]
	set RDS [expr ([clock microseconds] - $real_launch_time) / 1000000. ]; # real duration of simulaion (in seconds)

	set tmp [open statistics/statfile.out a]
	set str "DS = $DS N = $N M = $M DW = $DW CW = $CW BW = $BW PD = $PD PT = $PT"
	set str "$str\nBL = $BL L = $L lambda = $lambda mu = $mu BS = $bs BR = $br BL = $bl"

	set str "$str\nDS            (s)= $DS"
	savestatistics "DS" $DS

	set str "$str\nRDS           (s)= $RDS"
	savestatistics "RDS" $RDS


	set BD_ [expr $bd / $br]
	set str "$str\nBD            (s)= $BD_"
	savestatistics "BD" $BD_
	
	set BQD_ [expr $bqd / $br]
	set str "$str\nBQD           (s)= $BQD_"
	savestatistics "BQD" $BQD_

	set BWT_ [expr $bwt / $br]
	set str "$str\nBWT           (s)= $BWT_"
	savestatistics "BWT" $BWT_

	set BURSTDURATION [expr $burstduration / $bs]
	set str "$str\nBURSTDURATION (s)= $BURSTDURATION"
	savestatistics "BURSTDURATION" $BURSTDURATION

	set BURSTSIZE [expr $tp / $br]
	set str "$str\nBURSTSIZE   (byte)= $BURSTSIZE"
	savestatistics "BURSTSIZE" $BURSTSIZE

	set BDP [expr ($bl) / ($bs + 0.0)]
	set str "$str\nBDP            (%)= $BDP"
	savestatistics "BDP" $BDP
			
	set bt [getdurationofburst $tp $BW] 
	set NT_ [expr $bt / ($DS*$N*$M*$DW)]
	set str "$str\nNT             (%)= $NT_"
	savestatistics "NT" $NT_

	set TP $tp
	set str "$str\nTP      (Bytes)= $TP"
	savestatistics "TP" $TP
	
	set BR [expr $br]
	set str "$str\nBR             = $BR"
	savestatistics "BR" $BR

	for {set i 0} {$i < $M} {incr i} {
		set str "$str\nBR($i)          = $brc($i)"
		savestatistics "BR$i" [expr $brc($i)]
		set btc [getdurationofburst $tpc($i) $BW]
		set str "$str\nNT($i)          = [expr $btc / ($DS*$N*$DW)]"
		savestatistics "NT$i" [expr $btc / ($DS*$N*$DW)]
		set str "$str\nBD($i)       (s)= $BD($i)"
		savestatistics "BD$i" $BD($i)
		set str "$str\nBQD($i)      (s)= $BQD($i)"
		savestatistics "BQD$i" $BQD($i)
		set str "$str\nBWT($i)      (s)= $BWT($i)"
		savestatistics "BWT$i" $BWT($i)
	}

	set CPS $cps
	set str "$str\nCPS             = $CPS"
	savestatistics "CPS" $CPS
	
	set CDP [expr $cl / ($cps + 0.0)]
	set str "$str\nCDP            (%)= $CDP"
	savestatistics "CDP" $CDP
	
	puts $str
	puts $tmp $str
	close $tmp
	
	puts "Simulation complete";
    exit 0
}

set str "N = $N M = $M DW = $DW CW = $CW PD = $PD L = $L lambda = $lambda"
puts $str
set trace [open statistics/tracefile.out a]
puts $trace $str
close $trace

# Simulation progress procedure show advance of simulation
proc simulationprogress {} {
	global ns num_of_bursts 
	set now [$ns now]
	set str "$now\t$num_of_bursts"
	puts $str
	set trace [open statistics/tracefile.out a]
	puts $trace $str
	close $trace
	# simulationprogress procedure is called recursively after 0.01seconds
	$ns at [expr $now + 0.01] "simulationprogress"
}

# Schedule the simulation
# The pod nodes are launched sequentially one after the other (0.1 microsecond = 0.0000001 second) to avoid the overload of controller node at the start of the simulation
set launchtime 0.0
$ns at $launchtime "simulationprogress"
for {set i 0} {$i < $N} {incr i} {
 	$ns at $launchtime "burstgenerator $i 0"
	set launchtime [expr $launchtime + 0.0000001] 
}

# $ns at 10.0 "finish"

# Start the simulation
$ns run

