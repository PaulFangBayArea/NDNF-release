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
 * This scenario simulates a network topology consisted of three consumers, one router, one NFV node, and three producers:
 * 
 * 
 * 
 *      +----------+     1Mbps      +------------+     1Mbps      +----------+ 
 *      | Node 0   | <------------> |            |<------------>  | Node 5   |
 *      +----------+         10ms   +            +         10ms   +----------+ 
 *                                  +            +
 *      +----------+     1Mbps      +            +     1Mbps      +----------+ 
 *      | Node 1   | <------------> |   Node 3   |<------------>  | Node 6   |
 *      +----------+         10ms   +            +         10ms   +----------+ 
 *                                  +            +
 *      +----------+     1Mbps      +            +     1Mbps      +----------+ 
 *      | Node 2   | <------------> |            |<------------>  | Node 7   |
 *      +----------+         10ms   +------------+         10ms   +----------+ 
 *                                        ^
                                    1Mbps |
                                          | 10ms
                                          | 
                                     +--------+ 
 *                                   |        |
                                     +        +
                                     + Node 4 +  
                                     +        +    
                                     +--------+       
 * Node 0: consumer /name0
 * Node 1: consumer /name1
 * Node 2: consumer /name2
 * Node 3: router
 * Node 4: NFV node
 * Node 4: producer /name0
 * Node 5: producer /name1
 * Node 6: producer /name2
 * 
 * Consumer requests data from producer with frequency 10 interests per second
 * (interests contain constantly increasing sequence number).
 *
 * For every received interest, producer replies with a data packet, containing
 * 1024 bytes of virtual payload.
 *
 * To run scenario and see what is happening, use the following command:
 *
 *     NS_LOG=ndn.Consumer:ndn.Producer ./waf --run=test-multconsumer
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
  nodes.Create(8);

  // Connecting nodes using four links
  PointToPointHelper p2p;
  p2p.Install(nodes.Get(0), nodes.Get(3));
  p2p.Install(nodes.Get(1), nodes.Get(3));
  p2p.Install(nodes.Get(2), nodes.Get(3));
  p2p.Install(nodes.Get(3), nodes.Get(4));
  p2p.Install(nodes.Get(3), nodes.Get(5));
  p2p.Install(nodes.Get(3), nodes.Get(6));
  p2p.Install(nodes.Get(3), nodes.Get(7));

  // Install NDN stack on all nodes
  ndn::StackHelper ndnHelper;
  ndnHelper.SetDefaultRoutes(true);
  ndnHelper.InstallAll();

  // Choosing forwarding strategy
  //ndn::StrategyChoiceHelper::InstallAll("/f/name1", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll("/name1", "/localhost/nfd/strategy/multicast");
  //ndn::StrategyChoiceHelper::InstallAll("/f/name2", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll("/name2", "/localhost/nfd/strategy/multicast");
  //ndn::StrategyChoiceHelper::InstallAll("/f/name3", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll("/name3", "/localhost/nfd/strategy/multicast");

  // Installing applications
  //Node 0: Consumer 
  ndn::AppHelper consumerHelper("ns3::ndn::ConsumerCbr");
  consumerHelper.SetPrefix("/f/name1");
  consumerHelper.SetAttribute("Frequency", StringValue("1")); // 10 interests a second
  consumerHelper.Install(nodes.Get(0));                        

  //Node 1: Consumer 
  consumerHelper.SetPrefix("/f/name2");
  consumerHelper.SetAttribute("Frequency", StringValue("1")); // 10 interests a second
  consumerHelper.Install(nodes.Get(1));    

  //Node 2: Consumer 
  consumerHelper.SetPrefix("/f/name3");
  consumerHelper.SetAttribute("Frequency", StringValue("1")); // 10 interests a second
  consumerHelper.Install(nodes.Get(2));   
                   
  //Node 4: NFV Node
  ndn::AppHelper app2("ns3::ndn::NFVNode");
  app2.SetAttribute("Prefix1", StringValue("/f"));
  app2.SetAttribute("Signature", UintegerValue(4));
  app2.Install(nodes.Get(4));

  //Node 5: Producer
  ndn::AppHelper producerHelper("ns3::ndn::Producer");
  producerHelper.SetPrefix("/name1");
  producerHelper.SetAttribute("PayloadSize", StringValue("1024"));
  producerHelper.SetAttribute("Signature",UintegerValue(5));
  producerHelper.Install(nodes.Get(5)); // Producer

  //Node 6: Producer
  producerHelper.SetPrefix("/name2");
  producerHelper.SetAttribute("PayloadSize", StringValue("1024"));
  producerHelper.SetAttribute("Signature", UintegerValue(6));
  producerHelper.Install(nodes.Get(6)); // Producer

  //Node 7: Producer
  producerHelper.SetPrefix("/name3");
  producerHelper.SetAttribute("PayloadSize", StringValue("1024"));
  producerHelper.SetAttribute("Signature", UintegerValue(7));
  producerHelper.Install(nodes.Get(7)); // Producer

  Ptr<Node> Node3 = nodes.Get(3);
  Ptr<Node> Node4 = nodes.Get(4);
  
  //create FIB
  ndn::FibHelper::AddRoute(Node3, "/f", Node4, 1); 
  
  PcapWriter trace("ndn-test-multiconsumer.pcap");
  Config::ConnectWithoutContext("/NodeList/*/DeviceList/*/$ns3::PointToPointNetDevice/MacTx",
                                MakeCallback(&PcapWriter::TracePacket, &trace));

  Simulator::Stop(Seconds(10.0));

  //obtain metrics
  ndn::L3RateTracer::InstallAll("ndn-test-multiconsumer-L3-trace.txt", Seconds(1));

  ndn::CsTracer::InstallAll("ndn-test-multiconsumer-Cs-trace.txt", Seconds(1));

  ndn::AppDelayTracer::InstallAll("ndn-test-multiconsumer-app-delays-trace.txt");
  
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
