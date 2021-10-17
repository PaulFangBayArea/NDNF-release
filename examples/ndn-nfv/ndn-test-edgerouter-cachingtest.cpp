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

// ndn-test.cpp

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/ndnSIM-module.h"

namespace ns3 {

/**
 * This scenario simulates a network topology consisted of a consumer,two routers, a NFV node, and a producer:
 * 
 * 
 * 
 * 
 *
 *
 *                                        consumer #1      NFV Node      
 *                                            |               |                                            
 *                                            |               |               
 *                                            |               |               
 *                       consumer #0<---->router #0<---->  router #1<---->  Edge router <----->  producer 
 * 
 * Node 0: consumer #0
 * Node 1: router #0
 * Node 2: consumer #1
 * Node 3: router #1
 * Node 4: NFV node
 * Node 5: edge router
 * Node 6: producer
 * Consumer requests data from producer with frequency 10 interests per second
 * (interests contain constantly increasing sequence number).
 *
 * For every received interest, producer replies with a data packet, containing
 * 1024 bytes of virtual payload.
 *
 * To run scenario and see what is happening, use the following command:
 *
 *     NS_LOG=ndn.Consumer:ndn.Producer ./waf --run=ndn-test-edgerouter-cachingtets
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
  Config::SetDefault("ns3::PointToPointNetDevice::DataRate", StringValue("10Mbps"));
  Config::SetDefault("ns3::PointToPointChannel::Delay", StringValue("10ms"));
  Config::SetDefault("ns3::QueueBase::MaxSize", StringValue("20p"));

  // Read optional command-line parameters (e.g., enable visualizer with ./waf --run=<> --visualize
  CommandLine cmd;
  cmd.Parse(argc, argv);

  // Creating nodes
  NodeContainer nodes;
  nodes.Create(7);

  // Connecting nodes using four links
  PointToPointHelper p2p;
  p2p.Install(nodes.Get(0), nodes.Get(1));
  p2p.Install(nodes.Get(1), nodes.Get(2));
  p2p.Install(nodes.Get(1), nodes.Get(3));
  p2p.Install(nodes.Get(3), nodes.Get(4));
  p2p.Install(nodes.Get(3), nodes.Get(5));
  p2p.Install(nodes.Get(5), nodes.Get(6));

 

  // Install NDN stack on all nodes
  ndn::StackHelper ndnHelper;
  ndnHelper.SetDefaultRoutes(true);
  ndnHelper.InstallAll();


  // Choosing forwarding strategy
 
  ndn::StrategyChoiceHelper::InstallAll("/f/name0", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll("/name0", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll("/edge/name0", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll("/secure/name0", "/localhost/nfd/strategy/multicast");
  

  // Installing applications
  
  ndn::AppHelper consumerHelper("ns3::ndn::ConsumerCbr");
  consumerHelper.SetPrefix("/name0");
  consumerHelper.SetAttribute("Frequency", StringValue("1")); // 10 interests a second
  consumerHelper.Install(nodes.Get(0));  

  ndn::AppHelper consumerHelper1("ns3::ndn::ConsumerCbr2");
  consumerHelper1.SetPrefix("/name0");
  consumerHelper1.SetAttribute("Frequency", StringValue("1")); // 10 interests a second
  consumerHelper1.Install(nodes.Get(2));   
  
  // NFV Node
  ndn::AppHelper app1("ns3::ndn::NFVNodePro");
  app1.SetAttribute("Prefix1", StringValue("/edge"));
  app1.SetAttribute("Prefix2", StringValue("/f"));
  app1.SetAttribute("Signature",UintegerValue(4));
  app1.Install(nodes.Get(4)); 
  
  // Edge Router
  ndn::AppHelper app2("ns3::ndn::EdgeRouter2");
  app2.SetAttribute("Prefix1", StringValue("/name0"));
  app2.SetAttribute("Prefix2", StringValue("/f"));
  app2.SetAttribute("Prefix3", StringValue("/edge"));
  app2.SetAttribute("Prefix4", StringValue("/secure"));
  app2.SetAttribute("Signature", UintegerValue(5));
  app2.Install(nodes.Get(5));

  // Producer
  ndn::AppHelper producerHelper("ns3::ndn::Producer");
  producerHelper.SetPrefix("/secure/name0");
  producerHelper.SetAttribute("PayloadSize", StringValue("1024"));
  producerHelper.SetAttribute("Signature",UintegerValue(6));
  producerHelper.Install(nodes.Get(6)); 


  Ptr<Node> Node1 = nodes.Get(1);
  Ptr<Node> Node2 = nodes.Get(2);
  Ptr<Node> Node3 = nodes.Get(3);
  Ptr<Node> Node4 = nodes.Get(4);
  Ptr<Node> Node5 = nodes.Get(5);

  ndn::FibHelper::AddRoute(Node5, "/f/name0", Node3, 1); 
  ndn::FibHelper::AddRoute(Node3, "/f/name0", Node4, 1);  
  ndn::FibHelper::AddRoute(Node4, "/edge/name0", Node3, 1); 
  ndn::FibHelper::AddRoute(Node3, "/edge/name0", Node5, 1); 

  PcapWriter trace("ndn-test-edgerouter.pcap");
  Config::ConnectWithoutContext("/NodeList/*/DeviceList/*/$ns3::PointToPointNetDevice/MacTx",
                                MakeCallback(&PcapWriter::TracePacket, &trace));

  
  Simulator::Stop(Seconds(5.0));

 

  ndn::L3RateTracer::InstallAll("ndn-test-edgerouter-cachingtest-L3-trace.txt", Seconds(1));

  ndn::CsTracer::InstallAll("ndn-test-edgerouter-cachingtest-Cs-trace.txt", Seconds(1));

  printPitFib(nodes);

 
  ndn::AppDelayTracer::InstallAll("ndn-test-edgerouter-cachingtest-app-delays-trace.txt");
  
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
