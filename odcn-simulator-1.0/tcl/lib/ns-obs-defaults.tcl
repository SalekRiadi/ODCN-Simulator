
# Burst Manager defaults
# BurstManager set burstTimeout_ 0.1
# BurstManager set offsetTime_ 0.000010
# BurstManager set delta_ 0.00001
# BurstManager set maxBurstSize_ 10000
# BurstManager set pcntguard_ 0
# BurstManager set debug_ 0

# classifier defaults
# Classifier/EdgeClassifier set address_ -1
Classifier/BaseClassifier set address_ -1 
Classifier/BaseClassifier set proc_time 0.000001
Classifier/OBSPort set address_ -1

#GMG -- added initialization for bhpProcTime in edge and core
#       classifiers, and for FDLdelay in OBSFiberDelayLink

Classifier/BaseClassifier/CoreClassifier set bhpProcTime 0.000002
Classifier/BaseClassifier/EdgeClassifier set bhpProcTime 0.000002
Classifier/BaseClassifier/ControllerClassifier set bhpProcTime 0.000002 ; #added by Salek Riadi
OBSFiberDelayLink set FDLdelay 0.000001
OBSFiberDelayLink set avoidReordering_ false ; #added by kkamo 2005-09-06

# I need this for testing the routing funda of Agent
Agent/IPKT set packetSize_ 64
Agent/IPKT set address_ -1

Agent/Controller set packetSize_ 64
Agent/Controller set address_ -1

# In order to enhance simulation performance, i minimize number of UDP Packets using the following commande (added by Salek Riadi)
Agent/UDP set packetSize_ 100000000

#Defaults for fdl scheduling (note that option_ and max_fdls_ are
#   also initialized to zero in fdl-scheduler.cc (in C++);
#   we initialize here as well to avoid OTcl warning messages
Classifier/BaseClassifier set fdldelay 0.000001
Classifier/BaseClassifier set nfdl 1
Classifier/BaseClassifier set option 0
Classifier/BaseClassifier set maxfdls 0

#Default option for electronic buffering at edge node
Classifier/BaseClassifier set ebufoption 0

#Defaults for Self Similar Traffic Model

# RandomVariable/Gamma set avg 1.0
# RandomVariable/Gamma set stdev 1.0
# RandomVariable/NegBinom set avg 1.0
# RandomVariable/NegBinom set sparm 0
# Application/Traffic/SelfSimilar set rate 1.0
# Application/Traffic/SelfSimilar set std_dev_inter_batch_time 1.0
# Application/Traffic/SelfSimilar set batchsize 1.0
# Application/Traffic/SelfSimilar set sb 0
# Application/Traffic/SelfSimilar set Ht 0.5
# Application/Traffic/SelfSimilar set Hb -0.5
# Application/Traffic/SelfSimilar set starttime 0.0
# Application/Traffic/SelfSimilar set stoptime 1.0
