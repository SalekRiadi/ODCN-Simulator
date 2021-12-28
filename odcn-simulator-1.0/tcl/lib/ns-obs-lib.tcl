
# return the next hop 
Simulator instproc getnexthop { src dest } {
    set r [ $self get-routelogic ]
    set ret [ $r lookup $src $dest ]
    return $ret
}

# return the hop-count
Simulator instproc nhops { src dest } {
    set r [ $self get-routelogic ]
    set ret [ $r nhops $src $dest ]
    return $ret
}

# construct a new core node
Simulator instproc create-core-node { totcore } {
    set n [$self node]
    set nodeid [$n id]

    set cl [new Classifier/BaseClassifier/CoreClassifier]
    $n set classifier_ $cl
    [$n set classifier_] set address_ $nodeid
    [$n set classifier_] set type_ 1

    # puts " create-core-node: node $nodeid created"
    return $n
}

# construct a new edge node 
Simulator instproc create-edge-node { totcores totedges } {
    set n [$self node]
    set nodeid [$n id]

    set cl [new Classifier/BaseClassifier/EdgeClassifier]
    $n set classifier_ $cl
    [$n set classifier_] set address_ $nodeid
    [$n set classifier_] set type_ 0

    set p0 [new Classifier/OBSPort]
    $p0 set address_ $nodeid
    set iagent [new Agent/IPKT]
    $iagent initiagent $totcores $totedges
    $p0 install-iagent $iagent

    # dump the default burst-params 
    #$iagent0 dumpburstdefaults

    $n set dmux_ $p0
    [$n set classifier_] install $nodeid $p0
    $n attach $iagent

    #puts " create-edge-node: node $nodeid created"
    return $n 
}

# construct a controller node (added by Salek Riadi) 
Simulator instproc create-controller-node { totcores totedges datawavelengths bandwidthperchannel propagationdelay proctime} {
    set n [$self node]
    set nodeid [$n id]

    set cl [new Classifier/BaseClassifier/ControllerClassifier]
    $n set classifier_ $cl
    [$n set classifier_] set address_ $nodeid
    [$n set classifier_] set type_ 2

    set p0 [new Classifier/OBSPort]
    $p0 set address_ $nodeid
    set cagent [new Agent/Controller]
    $cagent initcontrolleragent $totcores $totedges $datawavelengths $bandwidthperchannel $propagationdelay $proctime
    $p0 install-controlleragent $cagent

    # dump the default burst-params 
    #$iagent0 dumpburstdefaults

    $n set dmux_ $p0
    [$n set classifier_] install $nodeid $p0
    $n attach $cagent

    puts " create-controller-node: node $nodeid created"
    puts "$totcores $totedges $datawavelengths $bandwidthperchannel $propagationdelay $proctime"
    return $n 
}


Simulator instproc create-ftp-connection { src des } {
    set source [new Agent/TCP]
    set sink [new Agent/TCPSink]
    $self attach-agent $src $source
    $self attach-agent $des $sink
    $self connect $source $sink
    
    set ftp [new Application/FTP]
    $ftp attach-agent $source

    return $ftp
}

# build the routing table
Simulator instproc build-routing-table { } {
    $self instvar Node_ link_
    set r [$self get-routelogic]

    $self compute-routes
    $self dump-routelogic-nh

    set n [Node set nn_]
    puts "The total number of nodes in this topology is $n"
    set i 0
    
    while { $i < $n } {
	if ![info exists Node_($i)] {
	    puts "Error node $i does not exist"
	    incr i 
	    continue 
	}
	set n1 $Node_($i) 
	set j 0
	while { $j < $n } {
#	    if { $i == $j } {
#		set p0 [new Classifier/Port]
#		$n1 set dmux_ $p0
#		[$n1 set classifier_] install $j $p0
#	    }
	    if { $i != $j } {
		set nh [$r lookup $i $j]
		if { $nh >=0 } {
		    puts "setting up route between $i $j with $nh"
		    [$n1 set classifier_] install $j [$link_($i:$nh) head]
		}
	    }
	    incr j
	}
	incr i
    }
}


# create a duplex
Simulator instproc createDuplexFiberLink { node1 node2 bwpc delay ncc ndc maxch } {
    $self instvar link_
   
    $self obs_duplex-FiberLink $node1 $node2 $bwpc $delay Null $maxch

    set id1 [$node1 id]
    set id2 [$node2 id]
    if [info exists link_($id1:$id2)] {
	[$node1 set classifier_] install-scheduler $id2 $ncc $ndc $maxch $bwpc
	puts "Adding schedulers between $node1 and $node2"
    }
    if [info exists link_($id2:$id1)] {
	[$node2 set classifier_] install-scheduler $id1 $ncc $ndc $maxch $bwpc
	puts "Adding schedulers between $node2 and $node1"
    }
}

# create a simplex - added by Salek Riadi
Simulator instproc createSimplexFiberLink { node1 node2 bwpc delay ncc ndc maxch } {
    $self instvar link_
   
    $self obs_simplex-FiberLink $node1 $node2 $bwpc $delay Null $maxch

    set id1 [$node1 id]
    set id2 [$node2 id]
    if [info exists link_($id1:$id2)] {
		[$node1 set classifier_] install-scheduler $id2 $ncc $ndc $maxch $bwpc
		puts "Adding schedulers between $node1 and $node2"
    }

}

#Create a node trace object, for tracing application traffic at edge
#nodes

Simulator instproc nodetrace-all { file } {
     set ndtr [ new NodeTrace ]
     $ndtr set trace_on 1
     $ndtr attach $file
}

#Flush the node trace data
Simulator instproc flush-nodetrace {} {
     set ndtr [ new NodeTrace ]
     $ndtr flush
}
