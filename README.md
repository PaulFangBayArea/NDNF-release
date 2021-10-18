Named Data Network Function Scenarios
====================================

This is the repo for running Named Data Network Function( NDNF) Scenarios using ndnSIM.


Feel free to contact pfang@umass.edu if you have any questions.

Prerequisites
=============
1. Core dependencies

    python >= 3.5

    libsqlite3

    pkg-config

    openssl

    Boost libraries >= 1.54


Following are the detailed steps for macOS to install the compiler, all necessary development tools and libraries, and ndn-cxx prerequisites.

    brew install boost pkg-config openssl libxml2

    export PKG_CONFIG_PATH="/usr/local/opt/libxml2/lib/pkgconfig"  


2. Dependencies for NS-3 Python bindings

If you are planning to use NS-3 python bindings, a number of additional dependencies should be installed. For example, in order to run visualizer module, the       following should be installed:

    brew install cairo goocanvas gtk+3 graphviz gobject-introspection castxml

    export PKG_CONFIG_PATH="$PKG_CONFIG_PATH:/usr/local/opt/libffi/lib/pkgconfig"  

    pip install pygraphviz pycairo PyGObject pygccxml




Install ndnSIM
==============

To install NS-3 and ndnSIM, please refer to http://ndnsim.net website.



Simulate NDNF Scenarios 
===================================


You can download the application files in  `apps` and the NDNF test scenarios in `examples`.


To run End-System NDNF scenario, you can use the following command:

    ./waf --run=ndn-test-singleconsumer

To run Edge-Router NDNF scenario, you can use the following command: 

    ./waf --run=ndn-test-edgerouter

To run Any-Router NDNF scenario, you can use the following command: 

    ./waf --run=ndn-test-anyrouter

To run End-System NDNF scenario with multiple NFV nodes, you can use the following command:   

    ./waf --run=ndn-test-multinfv

To run the simulation scenarios with the logging module of NS-3 enabled: 

    NS_LOG=ndn.Producer:ndn.Consumer ./waf --run=<scenario_name>

To run the simulation scenarios with visualizer:

    ./waf --run=<scenario_name> --vis

References
=========
We ask academic works, which are built on this code, to reference the following paper:

P. Fang and T. Wolf, "Enabling Virtual Network Functions in Named Data Networking," In Proc. of the Global Internet Symposium (GIS) in conjunction with the IEEE Conference on Computer Communications (INFOCOM), May 2021, pp.1–6
