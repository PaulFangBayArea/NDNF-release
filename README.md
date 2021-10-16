# Enabling Virtual Network Functions in Named Data Networking
Virtual network functions have been explored extensively for connection-oriented networks, but rarely for content-centric/information-centric networks. Our work is the first effort to enable virtual network functions in Named Data Networking.


Prerequisites
=============
1. Core dependencies
python >= 3.5
libsqlite3
pkg-config
openssl
Boost libraries >= 1.54

** macOS with HomeBrew:
brew install boost pkg-config openssl libxml2
export PKG_CONFIG_PATH="/usr/local/opt/libxml2/lib/pkgconfig"   # put this line in ~/.bashrc or manually type before ./waf configure

2. Dependencies for NS-3 Python bindings

** macOS with HomeBrew
brew install cairo goocanvas gtk+3 graphviz gobject-introspection castxml

export PKG_CONFIG_PATH="$PKG_CONFIG_PATH:/usr/local/opt/libffi/lib/pkgconfig"  # this needed only for running the next line
pip install pygraphviz pycairo PyGObject pygccxml

For more information how to install NS-3 and ndnSIM, please refer to http://ndnsim.net website.


Install ndnSIM
==============
ndnSIM package consists of three pieces:

* a custom branch of NS-3 that contains a few useful patches
* a customized python binding generation library (necessary if you want to use NS-3’s python bindings and/or visualizer module)
* the source code of ndnSIM module
* modified source code of ndn-cxx library and NDN Forwarding Daemon (NFD), attached to ndnSIM git repository as git submodules

The following commands download all pieces from GitHub repositories:

mkdir ndnSIM
cd ndnSIM
git clone https://github.com/named-data-ndnSIM/ns-3-dev.git ns-3
git clone https://github.com/named-data-ndnSIM/pybindgen.git pybindgen
git clone --recursive https://github.com/named-data-ndnSIM/ndnSIM.git ns-3/src/ndnSIM



Compiling
=========

To configure in optimized mode without logging **(default)**:

    ./waf configure

To configure in optimized mode with scenario logging enabled (logging in NS-3 and ndnSIM modules will still be disabled,
but you can see output from NS_LOG* calls from your scenarios and extensions):

    ./waf configure --logging

To configure in debug mode with all logging enabled

    ./waf configure --debug

If you have installed NS-3 in a non-standard location, you may need to set up ``PKG_CONFIG_PATH`` variable.
For example, if NS-3 is installed in /usr/local/, then the following command should be used to
configure scenario

    PKG_CONFIG_PATH=/usr/local/lib/pkgconfig ./waf configure

or

    PKG_CONFIG_PATH=/usr/local/lib/pkgconfig ./waf configure --logging

or

    PKG_CONFIG_PATH=/usr/local/lib/pkgconfig ./waf configure --debug

Running
=======

Normally, you can run scenarios either directly

    ./build/<scenario_name>

or using waf

    ./waf --run <scenario_name>

If NS-3 is installed in a non-standard location, on some platforms (e.g., Linux) you need to specify ``LD_LIBRARY_PATH`` variable:

    LD_LIBRARY_PATH=/usr/local/lib ./build/<scenario_name>

or

    LD_LIBRARY_PATH=/usr/local/lib ./waf --run <scenario_name>

To run scenario using debugger, use the following command:

    gdb --args ./build/<scenario_name>


Running with visualizer
-----------------------

There are several tricks to run scenarios in visualizer.  Before you can do it, you need to set up environment variables for python to find visualizer module.  The easiest way to do it using the following commands:

    cd ns-dev/ns-3
    ./waf shell

After these command, you will have complete environment to run the vizualizer.

The following will run scenario with visualizer:

    ./waf --run <scenario_name> --vis

or

    PKG_LIBRARY_PATH=/usr/local/lib ./waf --run <scenario_name> --vis

If you want to request automatic node placement, set up additional environment variable:

    NS_VIS_ASSIGN=1 ./waf --run <scenario_name> --vis

or

    PKG_LIBRARY_PATH=/usr/local/lib NS_VIS_ASSIGN=1 ./waf --run <scenario_name> --vis

Available simulations
=====================

To run scenario:
   ./waf --run=ndn-AnyRouter





## References

We ask academic works, which built on this code, to reference the following paper:

[Enabling Virtual Network Functions in Named Data Networking](https://ieeexplore.ieee.org/document/9484495)
Puming Fang and Tilman Wolf. In Proc. of the Global Internet Symposium (GIS) in conjunction with the IEEE Conference on Computer Communications (INFOCOM), May. 2021.
