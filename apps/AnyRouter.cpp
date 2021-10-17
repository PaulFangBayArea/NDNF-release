//Author: Puming Fang
//Network Systems Lab, UMass Amherst


#include "AnyRouter.hpp"
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

NS_LOG_COMPONENT_DEFINE("ndn.AnyRouter");

namespace ns3 {
  namespace ndn {
NS_OBJECT_ENSURE_REGISTERED(AnyRouter);


// register NS-3 type
TypeId
AnyRouter::GetTypeId()
{
  static TypeId tid = TypeId("ns3::ndn::AnyRouter").SetParent<App>().AddConstructor<AnyRouter>()
      .AddAttribute("Prefix1", "Prefix, for possible incoming Interest", StringValue("/"),
                    MakeNameAccessor(&AnyRouter::m_prefix1), MakeNameChecker())
      .AddAttribute("Prefix2", "Prefix, Prefix, for possible incoming Data", StringValue("/"),
                    MakeNameAccessor(&AnyRouter::m_prefix2), MakeNameChecker())
      .AddAttribute("Freshness", "Freshness of data packets, if 0, then unlimited freshness",
                    TimeValue(Seconds(0)), MakeTimeAccessor(&AnyRouter::m_freshness),
                    MakeTimeChecker())
      .AddAttribute(
         "Signature",
         "Fake signature, 0 valid signature (default), other values application-specific",
         UintegerValue(0), MakeUintegerAccessor(&AnyRouter::m_signature),
         MakeUintegerChecker<uint32_t>())
      .AddAttribute("KeyLocator",
                    "Name to be used for key locator.  If root, then key locator is not used",
                    NameValue(), MakeNameAccessor(&AnyRouter::m_keyLocator), MakeNameChecker());;
  return tid;
}

AnyRouter::AnyRouter()
{
  NS_LOG_FUNCTION_NOARGS();
}
// process upon start of the application
void
AnyRouter::StartApplication()
{
  NS_LOG_FUNCTION_NOARGS();
  // initialize ndn::App
  ndn::App::StartApplication();
 
  // add entry to FIB 
  
  FibHelper::AddRoute(GetNode(), m_prefix1, m_face, 0);
  //FibHelper::AddRoute(GetNode(), "/f/name0", m_face, 0);
  
}

// process when application is stopped
void
AnyRouter::StopApplication()
{
  // cleanup ndn::App
  ndn::App::StopApplication();
}

// callback that will be called when Data arrives
void
AnyRouter::OnData(std::shared_ptr<const ndn::Data> data)
{
  if (!m_active)
    return;

  App::OnData(data); // tracing inside
  //NS_LOG_FUNCTION(this << data);
  //NS_LOG_INFO("Receiving Data packet for " << data->getName());
  NS_LOG_INFO("Intermediate Router (" << GetNode()->GetId() << ") receives Data, Data = " << data->getName());
  // NS_LOG_INFO("Intermediate Router (" << GetNode()->GetId() << ") receives Data, Data = " << data->getName()<<"; Data is signed by = " << data->getSignature().getValue());
  AnyRouter::SendData(data);

  }

/////////////////////////////////////
//   Sending Data packet out   //
/////////////////////////////////////
void
AnyRouter::SendData(std::shared_ptr<const ndn::Data> contentObject)
{ 

  //receive Data /f/name0

  if(contentObject->getName().getPrefix(1)==m_prefix2){

  //NS_LOG_INFO("IntermediateRouter (" << GetNode()->GetId() << ") receives Data, Data = " << contentObject->getName()<<"; Data is signed by = " << contentObject->getSignature().getValue());
     
  //remove the prefix /f
  shared_ptr<Name> nameWithSequence = make_shared<Name>(contentObject->getName().getSubName(1,3));
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
 
  NS_LOG_INFO("Intermediate Router (" << GetNode()->GetId() << ") sends Data, Data= " << data->getName());
  //std::cout << "> IntermediateRouter (" << GetNode()->GetId() << ") sends Data, Data = " << data->getName()<<std::endl;

  // create real wire encoding
  data->wireEncode();

  //send Data
  m_transmittedDatas(data, this, m_face);
  m_appLink->onReceiveData(*data);
   }
}

// Callback that will be called when Interest arrives
void
AnyRouter::OnInterest(std::shared_ptr<const ndn::Interest> interest)
{
  ndn::App::OnInterest(interest);
  NS_LOG_INFO("Intermediate Router (" << GetNode()->GetId() << ") receives Interest, Interest = " << interest->getName());
  //uint32_t seq = interest->getName().at(-1).toSequenceNumber();
  //std::cout<<"Interest SequenceNumber = "<<seq<<std::endl;

  //NS_LOG_DEBUG("Received Interest packet for " << interest->getName());

  if(interest->getName().getPrefix(1)==m_prefix1)
  {
  Simulator::Schedule(Seconds(0), &AnyRouter::SendInterest, this);
  }

}

/////////////////////////////////////
//   Sending Interest packet out   //
/////////////////////////////////////
void
AnyRouter::SendInterest()
{
  //look up PIT
  auto& nodePit = GetNode()->GetObject<ndn::L3Protocol>()->getForwarder()->getPit();
  nfd::Pit::const_iterator iter;
  for (iter = nodePit.begin(); iter != nodePit.end(); iter++) {

  //if PIT entry starts with /name0
  if(iter->getName().getPrefix(1)==m_prefix1){
  //std::cout << "> IntermediateRouter (" << GetNode()->GetId() << ") fetches PIT entry, PIT entry = " << iter->getName()<<std::endl;

  //addd prefix /f
  std::string nfv= "/f";
  shared_ptr<Name> nameWithSequence = make_shared<Name>(iter->getName().toUri().insert(0,nfv));
  shared_ptr<Interest> interest = make_shared<Interest>();
  interest->setName(*nameWithSequence);
  interest->setCanBePrefix(false);
  Ptr<UniformRandomVariable> rand = CreateObject<UniformRandomVariable>();
  interest->setNonce(rand->GetValue(0, std::numeric_limits<uint32_t>::max()));
  interest->setInterestLifetime(ndn::time::seconds(1));
  
  //NS_LOG_DEBUG("Sending Interest packet for " << *interest);
  NS_LOG_INFO("Intermediate Router (" << GetNode()->GetId() << ") sends Interest, Interest = " << interest->getName());
  //std::cout<< "> IntermediateRouter (" << GetNode()->GetId() << ") sends Interest, Interest = " << interest->getName()<<std::endl;

  // send Interest
  m_transmittedInterests(interest, this, m_face);

  m_appLink->onReceiveInterest(*interest);
  }

  }
}


} // namespace ndn
} // namespace ns3
