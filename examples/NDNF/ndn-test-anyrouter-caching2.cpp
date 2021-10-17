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
 * 
 * 
 * 
 *
 *                                                  NFV Node      
 *                                                     |                                            
 *                                                     |               
 *                                                     |               
 *             consumer #0<---->router #0 <---->  router #1 <----> Inner Edge router <----> Outer Edge router <-----> producer 
 * 
 * Node 0: consumer #1
 * Node 1: router #0
 * Node 2: router #1
 * Node 3: NFV node
 * Node 4: Inner Edge router
 * Node 5: Outer Edge router
 * Node 6: producer
 * Node 7: consumer #2
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

  //Topology
  AnnotatedTopologyReader topologyReader("", 25);
  topologyReader.SetFileName("src/ndnSIM/examples/topologies/ndn-test-anyrouter-caching2.txt");
  topologyReader.Read();

  // Install NDN stack on all nodes
  ndn::StackHelper ndnHelper;
  ndnHelper.SetDefaultRoutes(true);
  ndnHelper.InstallAll();


  // Choosing forwarding strategy
  ndn::StrategyChoiceHelper::InstallAll("/name", "/localhost/nfd/strategy/multicast");
  
  // Installing applications

  // Getting containers for the consumer/producer
  Ptr<Node> Consumer = Names::Find<Node>("Consumer");
  Ptr<Node> Router0 = Names::Find<Node>("Router0");
  Ptr<Node> Router1 = Names::Find<Node>("Router1");
  Ptr<Node> NFV = Names::Find<Node>("NFV");
  Ptr<Node> InnerER = Names::Find<Node>("InnerER");
  Ptr<Node> OuterER = Names::Find<Node>("OuterER");
  Ptr<Node> Producer = Names::Find<Node>("Producer");
  Ptr<Node> Consumer2 = Names::Find<Node>("Consumer2");


  //Node 7: Consumer2
  ndn::AppHelper consumerHelper1("ns3::ndn::ConsumerCbr2");
  consumerHelper1.SetPrefix("/name");
  consumerHelper1.SetAttribute("Frequency", StringValue("1")); // 10 interests a second
  consumerHelper1.Install(Consumer2);   
  
  //Node 0: Consumer
  ndn::AppHelper consumerHelper("ns3::ndn::ConsumerCbr");
  consumerHelper.SetPrefix("/name");
  consumerHelper.SetAttribute("Frequency", StringValue("1")); // 10 interests a second
  consumerHelper.Install(Consumer);                       
   
  // Node 1: Intermediate Router
  ndn::AppHelper app0("ns3::ndn::AnyRouter");
  app0.SetAttribute("Prefix1", StringValue("/name"));
  app0.SetAttribute("Prefix2", StringValue("/f"));
  app0.SetAttribute("Signature",UintegerValue(1));
  app0.Install(Router0); 

  // Node 2: Intermediate Router
  app0.SetAttribute("Prefix1", StringValue("/name"));
  app0.SetAttribute("Prefix2", StringValue("/f"));
  app0.SetAttribute("Signature",UintegerValue(2));
  app0.Install(Router1); 

  //Node 3:NFV Node
  ndn::AppHelper app1("ns3::ndn::NFVNodePro");
  app1.SetAttribute("Prefix1", StringValue("/edge"));
  app1.SetAttribute("Prefix2", StringValue("/f"));
  app1.SetAttribute("Signature",UintegerValue(3));
  app1.Install(NFV); 
  
  //Node 4: Inner Edge Router
  ndn::AppHelper app2("ns3::ndn::InnerEdgeRouter");
  app2.SetAttribute("Prefix1", StringValue("/name"));
  app2.SetAttribute("Prefix2", StringValue("/f"));
  app2.SetAttribute("Prefix3", StringValue("/edge"));
  app2.SetAttribute("Prefix4", StringValue("/s"));
  app2.SetAttribute("Signature", UintegerValue(4));
  app2.Install(InnerER);

  //Node 5: Outer Edge Router
  ndn::AppHelper app3("ns3::ndn::OuterEdgeRouter");
  app3.SetAttribute("Prefix1", StringValue("/s"));
  app3.SetAttribute("Prefix2", StringValue("/name"));
  app3.SetAttribute("Signature", UintegerValue(5));
  app3.Install(OuterER);

  //Node 6: Producer
  ndn::AppHelper producerHelper("ns3::ndn::Producer");
  producerHelper.SetPrefix("/name");
  producerHelper.SetAttribute("PayloadSize", StringValue("1024"));
  producerHelper.SetAttribute("Signature",UintegerValue(6));
  producerHelper.Install(Producer); 


  ndn::FibHelper::AddRoute(Router0, "/f", Router1, 1); 
  ndn::FibHelper::AddRoute(Router1, "/f", NFV, 1);  
  //ndn::FibHelper::AddRoute(NFV, "/edge", Router1, 1); 
  ndn::FibHelper::AddRoute(Router1, "/edge", InnerER, 1); 
  ndn::FibHelper::AddRoute(InnerER, "/s",OuterER, 1); 
  ndn::FibHelper::AddRoute(OuterER, "/name",Producer, 1); 
 
  PcapWriter trace("ndn-test-anyrouter.pcap");
  Config::ConnectWithoutContext("/NodeList/*/DeviceList/*/$ns3::PointToPointNetDevice/MacTx",
                                MakeCallback(&PcapWriter::TracePacket, &trace));

  
  Simulator::Stop(Seconds(3.0));

 

  ndn::L3RateTracer::InstallAll("ndn-test-anyrouter-L3-trace.txt", Seconds(1));

  ndn::CsTracer::InstallAll("ndn-test-anyrouter-Cs-trace.txt", Seconds(1));

  

 
  ndn::AppDelayTracer::InstallAll("ndn-test-anyrouter-app-delays-trace.txt");
  
  Simulator::Run();
  //printPitFib(nodes);
  Simulator::Destroy();

 

  return 0;
}

} // namespace ns3

int
main(int argc, char* argv[])
{
  return ns3::main(argc, argv);
}
