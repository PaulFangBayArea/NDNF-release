//Author: Puming Fang
//Network Systems Lab, UMass Amherst


#include "EdgeRouter.hpp"
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

NS_LOG_COMPONENT_DEFINE("EdgeRouter");

namespace ns3 {
  namespace ndn {
NS_OBJECT_ENSURE_REGISTERED(EdgeRouter);


// register NS-3 type
TypeId
EdgeRouter::GetTypeId()
{
  static TypeId tid = TypeId("ns3::ndn::EdgeRouter").SetParent<App>().AddConstructor<EdgeRouter>()
      .AddAttribute("Prefix1", "Prefix, for which producer has the data", StringValue("/"),
                    MakeNameAccessor(&EdgeRouter::m_prefix1), MakeNameChecker())
      .AddAttribute("Prefix2", "Prefix, for which producer has the data", StringValue("/"),
                    MakeNameAccessor(&EdgeRouter::m_prefix2), MakeNameChecker())
      .AddAttribute("Prefix3", "Prefix, for which producer has the data", StringValue("/"),
                    MakeNameAccessor(&EdgeRouter::m_prefix3), MakeNameChecker())
      .AddAttribute("Freshness", "Freshness of data packets, if 0, then unlimited freshness",
                    TimeValue(Seconds(0)), MakeTimeAccessor(&EdgeRouter::m_freshness),
                    MakeTimeChecker())
      .AddAttribute(
         "Signature",
         "Fake signature, 0 valid signature (default), other values application-specific",
         UintegerValue(0), MakeUintegerAccessor(&EdgeRouter::m_signature),
         MakeUintegerChecker<uint32_t>())
      .AddAttribute("KeyLocator",
                    "Name to be used for key locator.  If root, then key locator is not used",
                    NameValue(), MakeNameAccessor(&EdgeRouter::m_keyLocator), MakeNameChecker());;
  return tid;
}

// Processing upon start of the application
void
EdgeRouter::StartApplication()
{
  // initialize ndn::App
  ndn::App::StartApplication();
 
  // Add entry to FIB 
  
  FibHelper::AddRoute(GetNode(), "/name0", m_face, 0);
  FibHelper::AddRoute(GetNode(), "/f/name0", m_face, 0);
 
}

// Processing when application is stopped
void
EdgeRouter::StopApplication()
{
  // cleanup ndn::App
  ndn::App::StopApplication();
}

void
EdgeRouter::SendInterest()
{

  auto& nodePit = GetNode()->GetObject<ndn::L3Protocol>()->getForwarder()->getPit();
  nfd::Pit::const_iterator iter;
  for (iter = nodePit.begin(); iter != nodePit.end(); iter++) {
  if(iter->getName().getPrefix(1)==m_prefix1){
  std::cout<<"EdgeRouter fetches PIT entry, PIT entry = " << iter->getName()<<std::endl;
  std::string nfv= "/f";
  shared_ptr<Name> nameWithSequence = make_shared<Name>(iter->getName().toUri().insert(0,nfv));
  shared_ptr<Interest> interest = make_shared<Interest>();
  interest->setName(*nameWithSequence);
  interest->setCanBePrefix(false);
  Ptr<UniformRandomVariable> rand = CreateObject<UniformRandomVariable>();
  interest->setNonce(rand->GetValue(0, std::numeric_limits<uint32_t>::max()));
  interest->setCanBePrefix(false);
  interest->setInterestLifetime(ndn::time::seconds(1));
  
  NS_LOG_DEBUG("Sending Interest packet for " << *interest);
  std::cout<<"EdgeRouter sends Interest, Interest = " << interest->getName()<<std::endl;

  // Call trace (for logging purposes)
  m_transmittedInterests(interest, this, m_face);

  m_appLink->onReceiveInterest(*interest);
  }

  }
}

void
EdgeRouter::SendData(std::shared_ptr<const ndn::Data> contentObject)
{ 
  

  if(contentObject->getName().getPrefix(1)==m_prefix2){
     
  std::cout << "EdgeRouter node receives Data, Data = " << contentObject->getName()<<"; Data is signed by = " << contentObject->getSignature().getValue()<<std::endl;
     
     
  shared_ptr<Name> nameWithSequence = make_shared<Name>(contentObject->getName().getSubName(1,3));
     
  auto data = std::make_shared<ndn::Data>();
  data->setName(*nameWithSequence);
  data->setFreshnessPeriod(::ndn::time::milliseconds(m_freshness.GetMilliSeconds()));

  data->setContent(contentObject->getContent());
  
  Signature signature;
  SignatureInfo signatureInfo(static_cast< ::ndn::tlv::SignatureTypeValue>(255));

  if (m_keyLocator.size() > 0) {
    signatureInfo.setKeyLocator(m_keyLocator);
  }

  signature.setInfo(signatureInfo);
  signature.setValue(::ndn::makeNonNegativeIntegerBlock(::ndn::tlv::SignatureValue, m_signature));

  data->setSignature(signature);
 
  NS_LOG_INFO("node(" << GetNode()->GetId() << ") responding with Data: " << data->getName());
  std::cout<<"EdgeRouter sends Data, Data = " << data->getName()<<std::endl;

  // to create real wire encoding
  data->wireEncode();

  m_transmittedDatas(data, this, m_face);
  m_appLink->onReceiveData(*data);
   }
}

// Callback that will be called when Interest arrives
void
EdgeRouter::OnInterest(std::shared_ptr<const ndn::Interest> interest)
{
  ndn::App::OnInterest(interest);
  std::cout << "EdgeRouter receives Interest, Interest = " << interest->getName()<< std::endl;

  NS_LOG_DEBUG("Received Interest packet for " << interest->getName());

  
  if(interest->getName().getPrefix(1)!="/localhost")
  {

    uint32_t seq = interest->getName().at(-1).toSequenceNumber();
    std::cout<<"Interest SequenceNumber = "<<seq<<std::endl;

    Simulator::Schedule(Seconds(0), &EdgeRouter::SendInterest, this);
  }

}

// Callback that will be called when Data arrives
void
EdgeRouter::OnData(std::shared_ptr<const ndn::Data> data)
{
  if (!m_active)
    return;

  App::OnData(data); // tracing inside
  NS_LOG_FUNCTION(this << data);
  NS_LOG_DEBUG("Receiving Data packet for " << data->getName());

  
  EdgeRouter::SendData(data);
 
  
  }
}
} // namespace ns3
