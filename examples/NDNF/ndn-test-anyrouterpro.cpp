//Puming Fang
//Network Systems Lab, UMass Amherst

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/ndnSIM-module.h"

namespace ns3 {

/**
 * This scenario simulates a network topology consisted of a consumer,two routers, a NFV node, and a producer:
 * 
 *                                        consumer #1      NFV Node      
 *                                            |               |                                            
 *                                            |               |               
 *                                            |               |               
 *      consumer #0<----> router #0<---->router #1<---->  router #2<---->  Inner Edge router <----> Outer Edge router <----->  producer 
 * 
 * Node 0: consumer #0
 * Node 1: router #0
 * Node 2: router #1
 * Node 3: consumer #1
 * Node 4: router #2
 * Node 5: NFV node
 * Node 6: Inner edge router
 * Node 7: Outer edge router
 * Node 8: producer
 * 
 * Consumer requests data from producer with frequency 10 interests per second
 * (interests contain constantly increasing sequence number).
 *
 * For every received interest, producer replies with a data packet, containing
 * 1024 bytes of virtual payload.
 *
 * To run scenario and see what is happening, use the following command:
 *
 *     NS_LOG=ndn.Consumer:ndn.Producer ./waf --run=ndn-test-anyrouter
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
  nodes.Create(9);

  PointToPointHelper p2p;
  p2p.Install(nodes.Get(0), nodes.Get(1));
  p2p.Install(nodes.Get(1), nodes.Get(2));
  p2p.Install(nodes.Get(2), nodes.Get(3));
  p2p.Install(nodes.Get(2), nodes.Get(4));
  p2p.Install(nodes.Get(4), nodes.Get(5));
  p2p.Install(nodes.Get(4), nodes.Get(6));
  p2p.Install(nodes.Get(6), nodes.Get(7));
  p2p.Install(nodes.Get(7), nodes.Get(8));

  // Install NDN stack on all nodes
  ndn::StackHelper ndnHelper;
  ndnHelper.SetDefaultRoutes(true);
  ndnHelper.InstallAll();


  // Choosing forwarding strategy
 
  //ndn::StrategyChoiceHelper::InstallAll("/f/name0", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll("/name0", "/localhost/nfd/strategy/multicast");
  //ndn::StrategyChoiceHelper::InstallAll("/edge/name0", "/localhost/nfd/strategy/multicast");
  //ndn::StrategyChoiceHelper::InstallAll("/secure/name0", "/localhost/nfd/strategy/multicast");
  

  // Installing applications
  
  

  //Node 0: Consumer
  ndn::AppHelper consumerHelper("ns3::ndn::ConsumerCbr");
  //consumerHelper.SetPrefix("/name0");
  //consumerHelper.SetAttribute("Frequency", StringValue("1")); // 10 interests a second
  //consumerHelper.Install(nodes.Get(0));   

  //Node 3: Consumer
  consumerHelper.SetPrefix("/name0");
  consumerHelper.SetAttribute("Frequency", StringValue("1")); // 10 interests a second
  consumerHelper.Install(nodes.Get(3));  

  
  // Node 5: NFV Node
  ndn::AppHelper app1("ns3::ndn::NFVT2");
  app1.SetAttribute("Prefix1", StringValue("/edge"));
  app1.SetAttribute("Prefix2", StringValue("/f"));
  app1.SetAttribute("Signature",UintegerValue(5));
  app1.Install(nodes.Get(5)); 
  
  // Node 6: Inner Edge Router
  ndn::AppHelper app2("ns3::ndn::InnerEdgeRouterPro");
  app2.SetAttribute("Prefix1", StringValue("/name0"));
  app2.SetAttribute("Prefix2", StringValue("/f"));
  app2.SetAttribute("Prefix3", StringValue("/edge"));
  app2.SetAttribute("Prefix4", StringValue("/secure"));
  app2.SetAttribute("Signature", UintegerValue(6));
  app2.Install(nodes.Get(6));
  
  // Node 7: Outer Edge Router
  ndn::AppHelper app0("ns3::ndn::OuterEdgeRouter");
  app0.SetAttribute("Prefix1", StringValue("/secure"));
  app0.SetAttribute("Prefix2", StringValue("/name0"));
  app0.SetAttribute("Signature", UintegerValue(7));
  app0.Install(nodes.Get(7));
  // Node 1: Intermediate Router
  ndn::AppHelper app3("ns3::ndn::AnyRouter");
  app3.SetAttribute("Prefix1", StringValue("/name0"));
  app3.SetAttribute("Prefix2", StringValue("/f"));
  app3.SetAttribute("Signature",UintegerValue(1));
  app3.Install(nodes.Get(1)); 

  // Node 2: Intermediate Router
  app3.SetAttribute("Prefix1", StringValue("/name0"));
  app3.SetAttribute("Prefix2", StringValue("/f"));
  app3.SetAttribute("Signature",UintegerValue(2));
  app3.Install(nodes.Get(2)); 

  // Node 4: Intermediate Router
  app3.SetAttribute("Prefix1", StringValue("/name0"));
  app3.SetAttribute("Prefix2", StringValue("/f"));
  app3.SetAttribute("Signature",UintegerValue(4));
  app3.Install(nodes.Get(4)); 

  // Node 7: Producer
  ndn::AppHelper producerHelper("ns3::ndn::Producer");
  producerHelper.SetPrefix("/name0");
  producerHelper.SetAttribute("PayloadSize", StringValue("1024"));
  producerHelper.SetAttribute("Signature",UintegerValue(8));
  producerHelper.Install(nodes.Get(8)); 

  
  Ptr<Node> Node1 = nodes.Get(1);
  Ptr<Node> Node2 = nodes.Get(2);
  Ptr<Node> Node3 = nodes.Get(3);
  Ptr<Node> Node4 = nodes.Get(4);
  Ptr<Node> Node5 = nodes.Get(5);
  Ptr<Node> Node6 = nodes.Get(6);
  Ptr<Node> Node7 = nodes.Get(7);
  Ptr<Node> Node8 = nodes.Get(8);
  //create FIB
  ndn::FibHelper::AddRoute(Node1, "/f", Node2, 1); 
  ndn::FibHelper::AddRoute(Node2, "/f", Node4, 1);  
  ndn::FibHelper::AddRoute(Node4, "/f", Node5, 1);
  ndn::FibHelper::AddRoute(Node5, "/edge", Node4, 1); 
  ndn::FibHelper::AddRoute(Node4, "/edge", Node6, 1); 
  ndn::FibHelper::AddRoute(Node6, "/secure", Node7, 1); 
  ndn::FibHelper::AddRoute(Node7, "/name0", Node8, 1); 
  

  PcapWriter trace("ndn-test-anyrouter.pcap");
  Config::ConnectWithoutContext("/NodeList/*/DeviceList/*/$ns3::PointToPointNetDevice/MacTx",
                                MakeCallback(&PcapWriter::TracePacket, &trace));

  
  Simulator::Stop(Seconds(2.0));

 
  //obtain metrics
  ndn::L3RateTracer::InstallAll("ndn-test-anyrouter-L3-trace.txt", Seconds(1));

  ndn::CsTracer::InstallAll("ndn-test-anyrouter-Cs-trace.txt", Seconds(1));

  ndn::AppDelayTracer::InstallAll("ndn-test-anyrouter-app-delays-trace.txt");
  
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
