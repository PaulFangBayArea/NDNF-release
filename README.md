# Enabling Virtual Network Functions in Named Data Networking
Virtual network functions, which are placed logically in the path of a network connection, implement important processing steps, such as content inspection, load balancing, etc. While these types of operations have been explored extensively for connection-oriented networks, but rarely for content-centric/information-centric networks. Our work is the first effort to enable virtual network functions in Named Data Networking. A variety of test scenarios that have been evaluated on the widely used ndnSIM simulator. We show that Named Data Network Functions can be implemented effectively and can be invoked by end systems or intermediate nodes to transparently implement network policies.


Prerequisites
=============
1. Core dependencies

    python >= 3.5

    libsqlite3

    pkg-config

    openssl

    Boost libraries >= 1.54


Following are the detailed steps for macOS to install the compiler, all necessary development tools and libraries, and ndn-cxx prerequisites.

* macOS with HomeBrew:
   
    brew install boost pkg-config openssl libxml2
  
    export PKG_CONFIG_PATH="/usr/local/opt/libxml2/lib/pkgconfig"  


2. Dependencies for NS-3 Python bindings

* macOS with HomeBrew

    brew install cairo goocanvas gtk+3 graphviz gobject-introspection castxml

    export PKG_CONFIG_PATH="$PKG_CONFIG_PATH:/usr/local/opt/libffi/lib/pkgconfig"  

    pip install pygraphviz pycairo PyGObject pygccxml




Install ndnSIM
==============
ndnSIM package consists of three pieces:

* a custom branch of NS-3 that contains a few useful patches
* a customized python binding generation library (necessary if you want to use NS-3’s python bindings and/or visualizer module)
* the source code of ndnSIM module
* modified source code of ndn-cxx library and NDN Forwarding Daemon (NFD), attached to ndnSIM git repository as git submodules

To install NS-3 and ndnSIM, please refer to http://ndnsim.net website.


Compile
=========

ndnSIM uses a standard NS-3 compilation procedure. Normally the following commands should be sufficient to configure and build ndnSIM with python bindings enabled:

    cd ndnSIM/ns-3
    ./waf configure --enable-examples
    ./waf


Simulate NDNF Scenarios using ndnSIM
===================================

You can run scenarios using the following command:

    ./waf --run=<scenario_name>

 To run End-System NDNF scenario, you can use the following command:
 
    ./waf --run=ndn-test-singleconsumer
    
 To run Edge-Router NDNF scenario, you can use the following command: 
   
    ./waf --run=ndn-test-edgerouter
    
 To run Any-Router NDNF scenario, you can use the following command: 
   
    ./waf --run=ndn-test-anyrouter
    
 To run End-System NDNF scenario with multiple NFV nodes, you can use the following command:   
 
    ./waf --run=ndn-test-multinfv
    
 To run the simulation scenarios with the logging module of NS-3 enabled (note that this will work only when NS-3 is compiled in debug mode): 
  
    NS_LOG=ndn.Producer:ndn.Consumer ./waf --run=<scenario_name>
 
 To run the simulation scenarios with visualizer:
 
    ./waf --run=<scenario_name> --vis
## References

We ask academic works, which built on this code, to reference the following paper:

[Enabling Virtual Network Functions in Named Data Networking](https://ieeexplore.ieee.org/document/9484495)
Puming Fang and Tilman Wolf. In Proc. of the Global Internet Symposium (GIS) in conjunction with the IEEE Conference on Computer Communications (INFOCOM), May. 2021.
