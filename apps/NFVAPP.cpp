//Author: Puming Fang
//Network Systems Lab, UMass Amherst


#include "NFVAPP.hpp"
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

NS_LOG_COMPONENT_DEFINE("NFVAPP");

namespace ns3 {
  namespace ndn {
NS_OBJECT_ENSURE_REGISTERED(NFVAPP);


// register NS-3 type
TypeId
NFVAPP::GetTypeId()
{
  static TypeId tid = TypeId("ns3::ndn::NFVAPP").SetParent<App>().AddConstructor<NFVAPP>()
      .AddAttribute("Freshness", "Freshness of data packets, if 0, then unlimited freshness",
                    TimeValue(Seconds(0)), MakeTimeAccessor(&NFVAPP::m_freshness),
                    MakeTimeChecker())
      .AddAttribute(
         "Signature",
         "Fake signature, 0 valid signature (default), other values application-specific",
         UintegerValue(0), MakeUintegerAccessor(&NFVAPP::m_signature),
         MakeUintegerChecker<uint32_t>())
      .AddAttribute("KeyLocator",
                    "Name to be used for key locator.  If root, then key locator is not used",
                    NameValue(), MakeNameAccessor(&NFVAPP::m_keyLocator), MakeNameChecker());;
  return tid;
}

// Processing upon start of the application
void
NFVAPP::StartApplication()
{
  // initialize ndn::App
  ndn::App::StartApplication();
 
  // Add entry to FIB 
  FibHelper::AddRoute(GetNode(), "/f", m_face, 0);
  //FibHelper::AddRoute(GetNode(), "/f/name1", m_face, 0);
  //FibHelper::AddRoute(GetNode(), "/f/name2", m_face, 0);
}

// Processing when application is stopped
void
NFVAPP::StopApplication()
{
  // cleanup ndn::App
  ndn::App::StopApplication();
}

void
NFVAPP::SendInterest()
{

  auto& nodePit = GetNode()->GetObject<ndn::L3Protocol>()->getForwarder()->getPit();
  nfd::Pit::const_iterator iter;
  for (iter = nodePit.begin(); iter != nodePit.end(); iter++) {


  //if PIT starts with /f
  if(iter->getName().getPrefix(1)=="/f"){
  std::cout<<"NFV node fetches PIT entry, PIT entry = " << iter->getName()<<std::endl;
  
  uint32_t seq = iter->getName().at(-1).toSequenceNumber();
  shared_ptr<Name> nameWithSequence = make_shared<Name>(iter->getName().getSubName(1,1));
  nameWithSequence->appendSequenceNumber(seq);
  shared_ptr<Interest> interest = make_shared<Interest>();
  interest->setName(*nameWithSequence);
  
  Ptr<UniformRandomVariable> rand = CreateObject<UniformRandomVariable>();
  interest->setNonce(rand->GetValue(0, std::numeric_limits<uint32_t>::max()));
  interest->setCanBePrefix(false);
  interest->setInterestLifetime(ndn::time::seconds(1));
  
  NS_LOG_DEBUG("Sending Interest packet for " << *interest);
  std::cout<<"NFV node sends Interest, Interest = " << interest->getName()<<std::endl;

  // Call trace (for logging purposes)
  m_transmittedInterests(interest, this, m_face);

  m_appLink->onReceiveInterest(*interest);
  }
}
}

void
NFVAPP::SendData(std::shared_ptr<const ndn::Data> contentObject)
{ 
  
  
   auto& nodePit = GetNode()->GetObject<ndn::L3Protocol>()->getForwarder()->getPit();
   nfd::Pit::const_iterator iter;
   iter=nodePit.begin();
  
  for (iter = nodePit.begin(); iter != nodePit.end(); iter++) {
  
  if(iter->getName().getSubName(1,2)==contentObject->getName().getPrefix(2))
  {

  auto data = std::make_shared<ndn::Data>();
  data->setName(iter->getName());
  std::cout<<"NFV node executes function f on Data" <<std::endl;
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
  std::cout<<"NFV node sends Data, Data = " << data->getName()<<std::endl;


  // to create real wire encoding
  data->wireEncode();

  m_transmittedDatas(data, this, m_face);
  m_appLink->onReceiveData(*data);
  }
  }
}

// Callback that will be called when Interest arrives
void
NFVAPP::OnInterest(std::shared_ptr<const ndn::Interest> interest)
{
  ndn::App::OnInterest(interest);
  std::cout << "NFV node receives Interest, Interest = " << interest->getName()<< std::endl;
  uint32_t seq = interest->getName().at(-1).toSequenceNumber();
  std::cout<<"Interest SequenceNumber = "<<seq<<std::endl;

  NS_LOG_DEBUG("Received Interest packet for " << interest->getName());

 
  
  //if Interest starting with /f is received.
  if(interest->getName().getPrefix(1)=="/f")
  {

    Simulator::Schedule(Seconds(0), &NFVAPP::SendInterest, this);
  }

}

// Callback that will be called when Data arrives
void
NFVAPP::OnData(std::shared_ptr<const ndn::Data> data)
{
  if (!m_active)
    return;

  App::OnData(data); // tracing inside
  NS_LOG_FUNCTION(this << data);
  NS_LOG_DEBUG("Receiving Data packet for " << data->getName());

  
  std::cout << "NFV node receives Data, Data = " << data->getName()<<"; Data is signed by = " << data->getSignature().getValue()<<std::endl;
  NFVAPP::SendData(data);
 
  
}
  } //namespace ndn
} // namespace ns3
