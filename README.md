Named Data Network Function Scenarios
====================================

This is the repo for running Named Data Network Function( NDNF) Scenarios using ndnSIM.




Prerequisites
=============
1. Core dependencies

    python >= 3.5

    libsqlite3

    pkg-config

    openssl

    Boost libraries >= 1.54


The following is to install the compiler, all necessary development tools and libraries, and ndn-cxx prerequisites for Ubuntu Linux 18.04.

    sudo apt install build-essential libsqlite3-dev libboost-all-dev libssl-dev git python-setuptools castxml
 


2. Dependencies for NS-3 Python bindings

To run visualizer module, you need to install additional dependencies.

   
    sudo apt install python-dev python-pygraphviz python-kiwi python-gnome2 ipython libcairo2-dev python3-gi libgirepository1.0-dev python-gi python-gi-cairo gir1.2-gtk-3.0 gir1.2-goocanvas-2.0 python-pip
    sudo pip install pygraphviz pycairo PyGObject pygccxml





Install ndnSIM
==============

To install NS-3 and ndnSIM, please refer to http://ndnsim.net.



Simulate NDNF Scenarios 
===================================


You can download the application files in  `apps`, the NDNF test scenarios in `examples`, and topology files in `topologies` as listed above, and then place the files in the `apps` and `examples` folder in the following directories respectively: '/ndnSIM/ns-3/src/ndnSIM/apps';  '/ndnSIM/ns-3/src/ndnSIM/examples'; '/ndnSIM/ns-3/src/ndnSIM/examples/topologies'.


To run End-System NDNF scenario, you can use the following command:

    ./waf --run=ndn-test-singleconsumer

To run Edge-Router NDNF scenario, you can use the following command: 

    ./waf --run=ndn-test-edgerouter

To run Any-Router NDNF scenario, you can use the following command: 

    ./waf --run=ndn-test-anyrouter
    
To run End-System NDNF scenario with multiple concurrent requests, you can use the following command:   

    ./waf --run=ndn-test-multiconsumer

To run End-System NDNF scenario with multiple NFV nodes, you can use the following command:   

    ./waf --run=ndn-test-multinfv

To run the simulation scenarios with the logging module of NS-3 enabled: 

    NS_LOG=ndn.Producer:ndn.Consumer:ndn.NFVNode ./waf --run=<scenario_name>


References
=========
We ask academic works, which are built on this code, to reference the following paper:

P. Fang and T. Wolf, "Enabling Virtual Network Functions in Named Data Networking," In Proc. of the Global Internet Symposium (GIS) in conjunction with the IEEE Conference on Computer Communications (INFOCOM), May 2021, pp.1–6
