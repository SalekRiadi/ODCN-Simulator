odcn-simulator-1.0 is an Optical DataCenter Networks Simulator that developed by Salek Riadi on the basis of OBS-ns.

OBS-ns was originally developed at Washington State University, Pullman, WA, and subsequently modified at University of Maryland, Baltimore County.

Installing odcn-simulator-1.0.

Step 1. Unzip the provided source in the "ns-allinone-2.x/ns-2.x" directory.

Step 2: Changes that need to be done to the existing ns-2 source

1. To introduce a new packet ptype namely PT_IPKT.
   (a) In the file "common/packet.h " add the following packet type PT_IPKT as shown below:

	typedef unsigned int packet_t;

	static const packet_t PT_TCP = 0;
	static const packet_t PT_UDP = 1;
	static const packet_t PT_CBR = 2;
	.
	.
	.
	// M-DART packets
	static const packet_t PT_MDART = 72;
	// PT_IPKT packets
	static const packet_t PT_IPKT = 73;  // insert PT_IPKT type here
	static packet_t       PT_NTYPE = 74; // This MUST be the LAST one


	p_info() {
		name_[PT_TCP]= "tcp";
		name_[PT_UDP]= "udp";
                ...........
 		name_[PT_TFRC]= "tcpFriend";
		name_[PT_TFRC_ACK]= "tcpFriendCtl";
		.
		.
		.
        name_[PT_LMS_SETUP]="LMS_SETUP";
        name_[PT_IPKT]="IPKT";  // add this line for IPKT
        name_[PT_NTYPE]= "undefined";
    }

    (b) In the file tcl/lib/ns-packet.tcl add

	set protolist {
		# Common:
			Common 
			Flags
			.
			.
			.
			AOMDV
		# OBS
			IPKT   # add this line for IPKT
		# Other:
			.
			.
			.
    }


Step 3: Changes to the ns-2 Makefile

	Add the following lines to the OBJ_CC tag as shown below
	OBJ_CC = \
			.
			.
			.
			odcn-simulator-1.0/cpp/debug.o \
			odcn-simulator-1.0/cpp/fiber-delay.o \
			odcn-simulator-1.0/cpp/agent/hdr_IPKT.o \
			odcn-simulator-1.0/cpp/agent/integrated_agent.o \
			odcn-simulator-1.0/cpp/agent/controller_agent.o \
			odcn-simulator-1.0/cpp/classifier/classifier-base.o \
			odcn-simulator-1.0/cpp/classifier/classifier-core.o \
			odcn-simulator-1.0/cpp/classifier/classifier-edge.o \
			odcn-simulator-1.0/cpp/classifier/classifier-obs-port.o \
			odcn-simulator-1.0/cpp/classifier/classifier-controller.o \
			odcn-simulator-1.0/cpp/scheduler/scheduler-group.o \
			odcn-simulator-1.0/cpp/scheduler/lauc-scheduler.o \
			odcn-simulator-1.0/cpp/scheduler/fdl-scheduler.o \
			odcn-simulator-1.0/cpp/scheduler/Table.o \
			odcn-simulator-1.0/cpp/scheduler/global-scheduler.o \
			odcn-simulator-1.0/cpp/scheduler/wavelength-scheduler.o \
			odcn-simulator-1.0/cpp/scheduler/wavelength-update.o \
			odcn-simulator-1.0/cpp/common/stat-collector.o \
			odcn-simulator-1.0/cpp/buffermanager/buffer-manager.o \
			$(OBJ_STL)


	Add the following lines to NS_TCL_LIB tag as shown below
	NS_TCL_LIB = \
			.
			.
			.
			odcn-simulator-1.0/tcl/lib/ns-obs-defaults.tcl \
			odcn-simulator-1.0/tcl/lib/ns-obs-lib.tcl \
			odcn-simulator-1.0/tcl/lib/ns-optic-link.tcl \
			$(NS_TCL_LIB_STL)


Step 4: Running the build.
	Run the following commands in the terminal:
	cd /usr/local/ns-allinone-2.x/ns-2.x
	make depend
	make
