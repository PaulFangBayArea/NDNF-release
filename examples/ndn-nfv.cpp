/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2011-2015  Regents of the University of California.
 *
 * This file is part of ndnSIM. See AUTHORS for complete list of ndnSIM authors and
 * contributors.
 *
 * ndnSIM is free software: you can redistribute it and/or modify it under the terms
 * of the GNU General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 *
 * ndnSIM is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * ndnSIM, e.g., in COPYING.md file.  If not, see <http://www.gnu.org/licenses/>.
 **/

// ndn-simple.cpp

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/ndnSIM-module.h"

namespace ns3 {

/**
 * This scenario simulates a network topology consisted of a consumer,two routers, a NFV node, and a producer:
                                 
                                    +-----------+ 
 *                                  |NFV Node #0|
                                    +-----------+
 *                                 1Mbps |
                                         | 10ms
                                         | 
 *      +----------+     1Mbps      +------------+     1Mbps     +------------+     1Mbps     +----------+ 
 *      | consumer | <------------> | router # 0 |<------------> | router # 1 |<------------> | producer |
 *      +----------+         10ms   +------------+         10ms  +------------+         10ms  +----------+ 
 *                                                                      ^
                                                                  1Mbps |
                                                                        | 10ms
                                                                        | 
                                                                   +-----------+ 
 *                                                                 |NFV Node #1|
                                                                   +-----------+
 * Node 0: consumer
 * Node 1: router #0
 * Node 2: router #1
 * Node 3: NFV node
 * Node 4: producer
 * Consumer requests data from producer with frequency 10 interests per second
 * (interests contain constantly increasing sequence number).
 *
 * For every received interest, producer replies with a data packet, containing
 * 1024 bytes of virtual payload.
 *
 * To run scenario and see what is happening, use the following command:
 *
 *     NS_LOG=ndn.Consumer:ndn.Producer ./waf --run=ndn-nfv
 */
 class PcapWriter {
public:
  PcapWriter(const std::string& file)
  {
    PcapHelper helper;
    m_pcap = helper.CreateFile(file, std::ios::out, PcapHelper::DLT_PPP);
  }

  void
  TracePacket(Ptr<const Packet> packet)
  {
    static PppHeader pppHeader;
    pppHeader.SetProtocol(0x0077);

    m_pcap->Write(Simulator::Now(), pppHeader, packet);
  }

private:
  Ptr<PcapFileWrapper> m_pcap;
};

void printPitFib(NodeContainer &nodes)
{
 //numNode variable is the number of nodes

std::cout << endl << "---- PIT ----"<< endl;
uint32_t numNodes = nodes.size();
    for (uint32_t i = 0; i < numNodes; i++) {
      std::cout << endl << "node(" << i << "):" << endl;
      auto node =  nodes.Get(i);
      auto& nodePit = node->GetObject<ndn::L3Protocol>()->getForwarder()->getPit();
      nfd::Pit::const_iterator iter;
      for (iter = nodePit.begin(); iter != nodePit.end(); iter++) {
        cout << iter->getName() << endl << " Period : " << iter->dataFreshnessPeriod << endl;
      }
    }

    std::cout << endl << "---- FIB ----"<< endl;

    for (uint32_t i = 0; i < numNodes; i++) {
      std::cout << endl << "node(" << i << "):" << endl;
      auto node =  nodes.Get(i);
      auto& nodePit = node->GetObject<ndn::L3Protocol>()->getForwarder()->getFib();
      nfd::Fib::const_iterator iter;
      for (iter = nodePit.begin(); iter != nodePit.end(); iter++) {
        {
        cout << iter->getPrefix();
        if(iter->hasNextHops())
        {
        auto& nextHops=  iter->getNextHops();
        nfd::fib::NextHopList::const_iterator ii;
        
              for(ii = nextHops.begin(); ii != nextHops.end(); ii++)
              cout << " : " << (unsigned) ii->getFace().getId() << endl;
        }
        }
      }
    }

      
}

int
main(int argc, char* argv[])
{
   ns3::PacketMetadata::Enable ();
  // setting default parameters for PointToPoint links and channels
  Config::SetDefault("ns3::PointToPointNetDevice::DataRate", StringValue("1Mbps"));
  Config::SetDefault("ns3::PointToPointChannel::Delay", StringValue("10ms"));
  Config::SetDefault("ns3::QueueBase::MaxSize", StringValue("20p"));

  // Read optional command-line parameters (e.g., enable visualizer with ./waf --run=<> --visualize
  CommandLine cmd;
  cmd.Parse(argc, argv);

  // Creating nodes
  NodeContainer nodes;
  nodes.Create(6);

  // Connecting nodes using four links
  PointToPointHelper p2p;
  p2p.Install(nodes.Get(0), nodes.Get(1));
  p2p.Install(nodes.Get(1), nodes.Get(2));
  p2p.Install(nodes.Get(1), nodes.Get(3));
  p2p.Install(nodes.Get(3), nodes.Get(4));
  p2p.Install(nodes.Get(3), nodes.Get(5));
  // Install NDN stack on all nodes
  ndn::StackHelper ndnHelper;
  ndnHelper.SetDefaultRoutes(true);
  ndnHelper.InstallAll();


  // Choosing forwarding strategy
   //ndn::StrategyChoiceHelper::InstallAll("/f1/name", "/localhost/nfd/strategy/multicast");
  //ndn::StrategyChoiceHelper::InstallAll("/f1/f2/f3/name", "/localhost/nfd/strategy/multicast");

  // Installing applications
  
   // Consumer will request /f/name/0, /f/name/1, ...
  ndn::AppHelper consumerHelper("ns3::ndn::ConsumerCbr");
  consumerHelper.SetPrefix("/f/name");
  consumerHelper.SetAttribute("Frequency", StringValue("0.1")); // 10 interests a second
  consumerHelper.Install(nodes.Get(0));                        // first node

  //consumerHelper.SetPrefix("/name");
  //consumerHelper.SetAttribute("Frequency", StringValue("1")); // 10 interests a second
  //consumerHelper.Install(nodes.Get(1));   
  //Consumer
  ndn::AppHelper consumerHelper1("ns3::ndn::NFVCCbr");
  consumerHelper1.SetPrefix("/name");
  consumerHelper1.SetAttribute("Frequency", StringValue("0.1")); // 10 interests a second
  consumerHelper1.Install(nodes.Get(2));                        // first node

 

  // Producer
  ndn::AppHelper producerHelper("ns3::ndn::Producer");

  //NFV Node
  // NFV node will reply to all requests starting with /f/name
  //producerHelper.SetPrefix("/name");
  //producerHelper.SetAttribute("PayloadSize", StringValue("1024"));
  //producerHelper.Install(nodes.Get(4)); // NFV Node

  //Producer will reply to all requests starting with /name

  producerHelper.SetPrefix("/name");
  producerHelper.SetAttribute("PayloadSize", StringValue("1024"));
  producerHelper.Install(nodes.Get(5)); // Producer

  //producerHelper.SetPrefix("/f/name");
  //producerHelper.SetAttribute("PayloadSize", StringValue("1024"));
  //producerHelper.Install(nodes.Get(5)); // Producer

  //ndn::AppHelper producerHelper1("NFV");
  // producerHelper1.SetPrefix("/f1/name");
  // producerHelper1.SetAttribute("PayloadSize", StringValue("1024"));
  // producerHelper1.Install(nodes.Get(4));


  //producerHelper1.SetPrefix("/f1/f2/name");
  //producerHelper1.SetAttribute("PayloadSize", StringValue("1024"));
  //producerHelper1.Install(nodes.Get(4));

  //ndn::AppHelper producerHelper1("NFVNODE");
  //producerHelper1.SetPrefix("/f/name");
  //producerHelper1.SetAttribute("PayloadSize", StringValue("1024"));
  //producerHelper1.Install(nodes.Get(5));

  //ndn::AppHelper producerHelper1("NFV");
  //producerHelper1.SetPrefix("/f1/f2/f3/name");
  //producerHelper1.SetAttribute("PayloadSize", StringValue("1024"));
  //producerHelper1.Install(nodes.Get(4));

  //ndn::GlobalRoutingHelper.AddOrigins("/f/name",nodes.Get(4));
  //ndn::GlobalRoutingHelper.AddOrigins("/name",nodes.Get(5));

  PcapWriter trace("ndn-simple-nfv.pcap");
  Config::ConnectWithoutContext("/NodeList/*/DeviceList/*/$ns3::PointToPointNetDevice/MacTx",
                                MakeCallback(&PcapWriter::TracePacket, &trace));

  
  Simulator::Stop(Seconds(20.0));

 

  ndn::L3RateTracer::InstallAll("nfv-node-L3-trace.txt", Seconds(1));

  ndn::CsTracer::InstallAll("nfv-node-Cs-trace.txt", Seconds(1));

  printPitFib(nodes);

 
  ndn::AppDelayTracer::InstallAll("ndn-nfv-app-delays-trace.txt");
  Simulator::Run();

  printPitFib(nodes);

  Simulator::Destroy();

 

  return 0;
}

} // namespace ns3

int
main(int argc, char* argv[])
{
  return ns3::main(argc, argv);
}
