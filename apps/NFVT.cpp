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

// custom-app.cpp


#include "NFVT.hpp"
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

NS_LOG_COMPONENT_DEFINE("NFVT");

namespace ns3 {
  namespace ndn {
NS_OBJECT_ENSURE_REGISTERED(NFVT);


// register NS-3 type
TypeId
NFVT::GetTypeId()
{
  static TypeId tid = TypeId("ns3::ndn::NFVT").SetParent<App>().AddConstructor<NFVT>()
      .AddAttribute("Prefix1", "Prefix, for which producer has the data", StringValue("/"),
                    MakeNameAccessor(&NFVT::m_prefix1), MakeNameChecker())
      .AddAttribute("Prefix2", "Prefix, for which producer has the data", StringValue("/"),
                    MakeNameAccessor(&NFVT::m_prefix2), MakeNameChecker())
      .AddAttribute("Freshness", "Freshness of data packets, if 0, then unlimited freshness",
                    TimeValue(Seconds(0)), MakeTimeAccessor(&NFVT::m_freshness),
                    MakeTimeChecker())
      .AddAttribute(
         "Signature",
         "Fake signature, 0 valid signature (default), other values application-specific",
         UintegerValue(0), MakeUintegerAccessor(&NFVT::m_signature),
         MakeUintegerChecker<uint32_t>())
      .AddAttribute("KeyLocator",
                    "Name to be used for key locator.  If root, then key locator is not used",
                    NameValue(), MakeNameAccessor(&NFVT::m_keyLocator), MakeNameChecker());;
  return tid;
}

// Processing upon start of the application
void
NFVT::StartApplication()
{
  // initialize ndn::App
  ndn::App::StartApplication();
 
  // Add entry to FIB 
  //FibHelper::AddRoute(GetNode(), m_prefix1, m_face, 0);
  //FibHelper::AddRoute(GetNode(), m_prefix2, m_face, 0);
  FibHelper::AddRoute(GetNode(), "/f/name0", m_face, 0);
  FibHelper::AddRoute(GetNode(), "/edge/name0", m_face, 0);
  //FibHelper::AddRoute(GetNode(), m_prefix, m_face, 0);
  // Schedule send of first interest
  //Simulator::Schedule(Seconds(0), &NFVT::SendInterest, this);
}

// Processing when application is stopped
void
NFVT::StopApplication()
{
  // cleanup ndn::App
  ndn::App::StopApplication();
}

void
NFVT::SendInterest()
{
  /////////////////////////////////////
  // Sending one Interest packet out //
  /////////////////////////////////////

  // Create and configure ndn::Interest
 
  //uint32_t seq = std::numeric_limits<uint32_t>::min(); // invalid
  auto& nodePit = GetNode()->GetObject<ndn::L3Protocol>()->getForwarder()->getPit();
  nfd::Pit::const_iterator iter;
  //iter=nodePit.begin();
  for (iter = nodePit.begin(); iter != nodePit.end(); iter++) {
  //std::cout<<"NFV node fetches PIT entry, PIT entry = " << iter->getName()<<std::endl;
  if(iter->getName().getPrefix(1)==m_prefix2){
  std::cout<<"NFV node fetches PIT entry, PIT entry = " << iter->getName()<<std::endl;
  //std::cout<<"TEST = " << iter->getName().getSubName(1,3)<<std::endl;
  //char Tunneling = (char) 'edge';
  std::string str = "/edge";
  shared_ptr<Name> nameWithSequence = make_shared<Name>(iter->getName().getSubName(1,2).toUri().insert(0,str));
  //shared_ptr<Name> nameWithSequence = make_shared<Name>(iter->getName().getSubName(1,4));
  //nameWithSequence->appendSequenceNumber(seq);
  shared_ptr<Interest> interest = make_shared<Interest>();
  //interest->setNonce(m_rand->GetValue(0, std::numeric_limits<uint32_t>::max()));
  interest->setName(*nameWithSequence);
  //interest->setCanBePrefix(false);
  //time::milliseconds interestLifeTime(m_interestLifeTime.GetMilliSeconds());
  //interest->setInterestLifetime(interestLifeTime);
  //auto interest = std::make_shared<ndn::Interest>("/name");
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
NFVT::SendData(std::shared_ptr<const ndn::Data> contentObject)
{ 
  if(contentObject->getName().getPrefix(1)==m_prefix1){
    //std::cout<<"NFV node receives Data /edge"<<std::endl;
    //char vnf = (char) '/f';
    std::string nfv= "/f";
  shared_ptr<Name> nameWithSequence = make_shared<Name>(contentObject->getName().getSubName(1,2).toUri().insert(0,nfv));
     //shared_ptr<Name> nameWithSequence = make_shared<Name>(contentObject->getName().getSubName(1,2));
       auto data = std::make_shared<ndn::Data>();
       data->setName(*nameWithSequence);
        data->setFreshnessPeriod(::ndn::time::milliseconds(m_freshness.GetMilliSeconds()));
  //data->setFreshnessPeriod(ndn::time::milliseconds(1000));
  
  //data->setContent(make_shared< ::ndn::Buffer>(m_virtualPayloadSize));
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

// Callback that will be called when Interest arrives
void
NFVT::OnInterest(std::shared_ptr<const ndn::Interest> interest)
{
  ndn::App::OnInterest(interest);
  std::cout << "NFV node receives Interest, Interest = " << interest->getName()<< std::endl;

  NS_LOG_DEBUG("Received Interest packet for " << interest->getName());

  // Note that Interests send out by the app will not be sent back to the app !
  //m_prefix2=/f

  if(interest->getName().getPrefix(1)==m_prefix2)
  {

    uint32_t seq = interest->getName().at(-1).toSequenceNumber();
    std::cout<<"Interest SequenceNumber = "<<seq<<std::endl;

    Simulator::Schedule(Seconds(0), &NFVT::SendInterest, this);
  }
  //auto data = std::make_shared<ndn::Data>(interest->getName());
  //data->setFreshnessPeriod(ndn::time::milliseconds(1000));
  //data->setContent(std::make_shared< ::ndn::Buffer>(1024));
  //ndn::StackHelper::getKeyChain().sign(*data);

  //NS_LOG_DEBUG("Sending Data packet for " << data->getName());

  // Call trace (for logging purposes)
  //m_transmittedDatas(data, this, m_face);

  //m_appLink->onReceiveData(*data);
}

// Callback that will be called when Data arrives
void
NFVT::OnData(std::shared_ptr<const ndn::Data> data)
{
  if (!m_active)
    return;

  App::OnData(data); // tracing inside
  NS_LOG_FUNCTION(this << data);
  NS_LOG_DEBUG("Receiving Data packet for " << data->getName());

  std::cout << "NFV node receives Data, Data = " << data->getName()<<"; Data is signed by = " << data->getSignature().getValue()<<std::endl;
  NFVT::SendData(data);
    //Parse Signature
  //auto buffer = data->getSignature().getValue().getBuffer();
  //Block signatureValueBlock = data->getSignature().getInfo(); 
  //signatureValueBlock.parse();
  //auto parsedSignatureBlock = signatureValueBlock.elements().begin();
  //uint64_t parsedSignatureValue = readNonNegativeInteger(*parsedSignatureBlock);
  //std::cout<<"Data Sig= " << parsedSignatureValue<<std::endl;
  
  //if(data->getName().getPrefix(1) != "/localhost"){
    
  //NFVT::SendData(data);

  //}
  
}
  }
} // namespace ns3
