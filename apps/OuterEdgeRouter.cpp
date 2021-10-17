//Author: Puming Fang
//Network Systems Lab, UMass Amherst


#include "OuterEdgeRouter.hpp"
#include "ns3/log.h"
#include "ns3/string.h"
#include "ns3/uinteger.h"
#include "ns3/packet.h"
#include "ns3/simulator.h"
#include "model/ndn-l3-protocol.hpp"
#include "helper/ndn-fib-helper.hpp"
#include "ns3/random-variable-stream.h"
#include <memory>

//
#include "ns3/ptr.h"
#include "ns3/callback.h"
#include "ns3/boolean.h"
#include "ns3/uinteger.h"
#include "ns3/integer.h"
#include "ns3/double.h"

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/ndnSIM-module.h"

NS_LOG_COMPONENT_DEFINE("ndn.OuterEdgeRouter");

namespace ns3 {
  namespace ndn {
NS_OBJECT_ENSURE_REGISTERED(OuterEdgeRouter);


// register NS-3 type
TypeId
OuterEdgeRouter::GetTypeId()
{
  static TypeId tid = TypeId("ns3::ndn::OuterEdgeRouter").SetParent<App>().AddConstructor<OuterEdgeRouter>()
      .AddAttribute("Prefix1", "Prefix, for possible incoming Interest", StringValue("/"),
                    MakeNameAccessor(&OuterEdgeRouter::m_prefix1), MakeNameChecker())
      .AddAttribute("Prefix2", "Prefix, for possible Incoming data", StringValue("/"),
                    MakeNameAccessor(&OuterEdgeRouter::m_prefix2), MakeNameChecker())
      .AddAttribute("Freshness", "Freshness of data packets, if 0, then unlimited freshness",
                    TimeValue(Seconds(0)), MakeTimeAccessor(&OuterEdgeRouter::m_freshness),
                    MakeTimeChecker())
      .AddAttribute(
         "Signature",
         "Fake signature, 0 valid signature (default), other values application-specific",
         UintegerValue(0), MakeUintegerAccessor(&OuterEdgeRouter::m_signature),
         MakeUintegerChecker<uint32_t>())
      .AddAttribute("KeyLocator",
                    "Name to be used for key locator.  If root, then key locator is not used",
                    NameValue(), MakeNameAccessor(&OuterEdgeRouter::m_keyLocator), MakeNameChecker());;
  return tid;
}

OuterEdgeRouter::OuterEdgeRouter()
{
  NS_LOG_FUNCTION_NOARGS();
}

// Processing upon start of the application
void
OuterEdgeRouter::StartApplication()
{
  NS_LOG_FUNCTION_NOARGS();
  // initialize ndn::App
  ndn::App::StartApplication();
 
  // Add entry to FIB 
  
  //FibHelper::AddRoute(GetNode(), "/name0", m_face, 0);
  FibHelper::AddRoute(GetNode(), m_prefix1, m_face, 0);
 
}

// Processing when application is stopped
void
OuterEdgeRouter::StopApplication()
{
  // cleanup ndn::App
  ndn::App::StopApplication();
}

void
OuterEdgeRouter::SendInterest()
{
 
  auto& nodePit = GetNode()->GetObject<ndn::L3Protocol>()->getForwarder()->getPit();
  nfd::Pit::const_iterator iter;
  for (iter = nodePit.begin(); iter != nodePit.end(); iter++) {
  //PIT entry starts with /secure
  if(iter->getName().getPrefix(1)==m_prefix1){
  //std::cout<<"Outer EdgeRouter receives Interest starting with /secure"<<std::endl;
  //std::cout<<"Outer EdgeRouter fetches PIT entry, PIT entry = " << iter->getName()<<std::endl;
  
  //remove /s
  shared_ptr<Name> nameWithSequence = make_shared<Name>(iter->getName().getSubName(1,3));
  shared_ptr<Interest> interest = make_shared<Interest>();
  interest->setName(*nameWithSequence);
  interest->setCanBePrefix(false);
  Ptr<UniformRandomVariable> rand = CreateObject<UniformRandomVariable>();
  interest->setNonce(rand->GetValue(0, std::numeric_limits<uint32_t>::max()));
  interest->setInterestLifetime(ndn::time::seconds(1));
  
  // NS_LOG_DEBUG("Sending Interest packet for " << *interest);
  NS_LOG_INFO("Outer EdgeRouter sends Interest, Interest = " << interest->getName());
  // std::cout<<"Outer EdgeRouter sends Interest, Interest = " << interest->getName()<<std::endl;

  // Call trace (for logging purposes)
  m_transmittedInterests(interest, this, m_face);

  m_appLink->onReceiveInterest(*interest);
  }
  
  }
}

void
OuterEdgeRouter::SendData(std::shared_ptr<const ndn::Data> contentObject)
{ 
  
  //OuterEdgeRouter receives Data /name0
  if(contentObject->getName().getPrefix(1)==m_prefix2){
  //NS_LOG_INFO("Outer EdgeRouter receives Data, Data = " << contentObject->getName()<<"; Data is signed by = " << contentObject->getSignature().getValue());  
  //std::cout << "Outer EdgeRouter receives Data, Data = " << contentObject->getName()<<"; Data is signed by = " << contentObject->getSignature().getValue()<<std::endl;
     
  //add prefix '/secure';
  std::string str = "/s";
  shared_ptr<Name> nameWithSequence = make_shared<Name>(contentObject->getName().toUri().insert(0,str));
  auto data = std::make_shared<ndn::Data>();
  data->setName(*nameWithSequence);
  data->setFreshnessPeriod(::ndn::time::milliseconds(m_freshness.GetMilliSeconds()));
  
  //pass content to the created Data
  data->setContent(contentObject->getContent());
  
  //sign the Data
  Signature signature;
  SignatureInfo signatureInfo(static_cast< ::ndn::tlv::SignatureTypeValue>(255));

  if (m_keyLocator.size() > 0) {
    signatureInfo.setKeyLocator(m_keyLocator);
  }

  signature.setInfo(signatureInfo);
  signature.setValue(::ndn::makeNonNegativeIntegerBlock(::ndn::tlv::SignatureValue, m_signature));

  data->setSignature(signature);
 
  NS_LOG_INFO("Outer EdgeRouter sends Data, Data = "<< data->getName());
  //std::cout<<"Outer EdgeRouter sends Data, Data = " << data->getName()<<std::endl;
  
  // to create real wire encoding
  data->wireEncode();

  m_transmittedDatas(data, this, m_face);
  m_appLink->onReceiveData(*data);
   }


}

// Callback that will be called when Interest arrives
void
OuterEdgeRouter::OnInterest(std::shared_ptr<const ndn::Interest> interest)
{
  ndn::App::OnInterest(interest);
  NS_LOG_INFO("Outer EdgeRouter receives Interest, Interest = " << interest->getName());
  //std::cout << "Outer EdgeRouter receives Interest, Interest = " << interest->getName()<< std::endl;
  //uint32_t seq = interest->getName().at(-1).toSequenceNumber();
  //std::cout<<"Interest SequenceNumber = "<<seq<<std::endl;
  //NS_LOG_DEBUG("Received Interest packet for " << interest->getName());

  //if Interest starts with /secure
  if(interest->getName().getPrefix(1)==m_prefix1)
  {

   

    Simulator::Schedule(Seconds(0), &OuterEdgeRouter::SendInterest, this);
  }

}

// Callback that will be called when Data arrives
void
OuterEdgeRouter::OnData(std::shared_ptr<const ndn::Data> data)
 {
  if (!m_active)
    return;

  App::OnData(data); // tracing inside
  NS_LOG_INFO("Outer EdgeRouter receives Data, Data = " << data->getName());  
  // NS_LOG_INFO("Outer EdgeRouter receives Data, Data = " << data->getName()<<"; Data is signed by = " << data->getSignature().getValue());
  //NS_LOG_FUNCTION(this << data);
  //NS_LOG_INFO("Outer EdgeRouter receives Data, Data= " << data->getName());

  OuterEdgeRouter::SendData(data);

  }
} // namespace ndn
} // namespace ns3
