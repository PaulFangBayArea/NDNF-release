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


#include "EdgeRouter2.hpp"
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

NS_LOG_COMPONENT_DEFINE("EdgeRouter2");

namespace ns3 {
  namespace ndn {
NS_OBJECT_ENSURE_REGISTERED(EdgeRouter2);


// register NS-3 type
TypeId
EdgeRouter2::GetTypeId()
{
  static TypeId tid = TypeId("ns3::ndn::EdgeRouter2").SetParent<App>().AddConstructor<EdgeRouter2>()
      .AddAttribute("Prefix1", "Prefix, for which producer has the data", StringValue("/"),
                    MakeNameAccessor(&EdgeRouter2::m_prefix1), MakeNameChecker())
      .AddAttribute("Prefix2", "Prefix, for which producer has the data", StringValue("/"),
                    MakeNameAccessor(&EdgeRouter2::m_prefix2), MakeNameChecker())
      .AddAttribute("Prefix3", "Prefix, for which producer has the data", StringValue("/"),
                    MakeNameAccessor(&EdgeRouter2::m_prefix3), MakeNameChecker())
      .AddAttribute("Prefix4", "Prefix, for which producer has the data", StringValue("/"),
                    MakeNameAccessor(&EdgeRouter2::m_prefix4), MakeNameChecker())
      .AddAttribute("Freshness", "Freshness of data packets, if 0, then unlimited freshness",
                    TimeValue(Seconds(0)), MakeTimeAccessor(&EdgeRouter2::m_freshness),
                    MakeTimeChecker())
      .AddAttribute(
         "Signature",
         "Fake signature, 0 valid signature (default), other values application-specific",
         UintegerValue(0), MakeUintegerAccessor(&EdgeRouter2::m_signature),
         MakeUintegerChecker<uint32_t>())
      .AddAttribute("KeyLocator",
                    "Name to be used for key locator.  If root, then key locator is not used",
                    NameValue(), MakeNameAccessor(&EdgeRouter2::m_keyLocator), MakeNameChecker());;
  return tid;
}

// Processing upon start of the application
void
EdgeRouter2::StartApplication()
{
  // initialize ndn::App
  ndn::App::StartApplication();
 
  // Add entry to FIB 
  
  FibHelper::AddRoute(GetNode(), "/name0", m_face, 0);
  FibHelper::AddRoute(GetNode(), "/f/name0", m_face, 0);
  FibHelper::AddRoute(GetNode(), "/edge/name0", m_face, 0);
  //FibHelper::AddRoute(GetNode(), m_prefix, m_face, 0);
  // Schedule send of first interest
  //Simulator::Schedule(Seconds(0), &EdgeRouter2::SendInterest, this);
}

// Processing when application is stopped
void
EdgeRouter2::StopApplication()
{
  // cleanup ndn::App
  ndn::App::StopApplication();
}

void
EdgeRouter2::SendInterest()
{
 
  auto& nodePit = GetNode()->GetObject<ndn::L3Protocol>()->getForwarder()->getPit();
  nfd::Pit::const_iterator iter;
  for (iter = nodePit.begin(); iter != nodePit.end(); iter++) {
  if(iter->getName().getPrefix(1)==m_prefix1){
  std::cout<<"EdgeRouter receives Interest starting with /name0"<<std::endl;
  std::cout<<"EdgeRouter fetches PIT entry, PIT entry = " << iter->getName()<<std::endl;
  std::string nfv= "/f";
  shared_ptr<Name> nameWithSequence = make_shared<Name>(iter->getName().toUri().insert(0,nfv));
  shared_ptr<Interest> interest = make_shared<Interest>();
  interest->setName(*nameWithSequence);
  interest->setCanBePrefix(false);
  Ptr<UniformRandomVariable> rand = CreateObject<UniformRandomVariable>();
  interest->setNonce(rand->GetValue(0, std::numeric_limits<uint32_t>::max()));
  interest->setInterestLifetime(ndn::time::seconds(1));
  
  NS_LOG_DEBUG("Sending Interest packet for " << *interest);
  std::cout<<"EdgeRouter sends Interest, Interest = " << interest->getName()<<std::endl;

  // Call trace (for logging purposes)
  m_transmittedInterests(interest, this, m_face);

  m_appLink->onReceiveInterest(*interest);
  }

  if(iter->getName().getPrefix(1)==m_prefix3){
  std::cout<<"EdgeRouter receives Interest starting with /edge"<<std::endl;
  std::cout<<"EdgeRouter fetches PIT entry, PIT entry = " << iter->getName()<<std::endl;
  std::string secure= "/secure";
  shared_ptr<Name> nameWithSequence = make_shared<Name>(iter->getName().getSubName(1,2).toUri().insert(0,secure));

  shared_ptr<Interest> interest = make_shared<Interest>();
  interest->setName(*nameWithSequence);
  interest->setCanBePrefix(false);
  Ptr<UniformRandomVariable> rand = CreateObject<UniformRandomVariable>();
  interest->setNonce(rand->GetValue(0, std::numeric_limits<uint32_t>::max()));
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
EdgeRouter2::SendData(std::shared_ptr<const ndn::Data> contentObject)
{ 
  
  //EdgeRouter2 receives Data /secure
  if(contentObject->getName().getPrefix(1)==m_prefix4){
     
  std::cout << "EdgeRouter receives Data, Data = " << contentObject->getName()<<"; Data is signed by = " << contentObject->getSignature().getValue()<<std::endl;
     
  //add prefix '/edge';
  std::string str = "/edge";
  shared_ptr<Name> nameWithSequence = make_shared<Name>(contentObject->getName().getSubName(1,3).toUri().insert(0,str));
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
 
  NS_LOG_INFO("node(" << GetNode()->GetId() << ") responding with Data: " << data->getName());
  std::cout<<"EdgeRouter sends Data, Data = " << data->getName()<<std::endl;
  
  // to create real wire encoding
  data->wireEncode();

  m_transmittedDatas(data, this, m_face);
  m_appLink->onReceiveData(*data);
   }

  //EdgeRouter2 receives Data /f
  if(contentObject->getName().getPrefix(1)==m_prefix2){
     
  std::cout << "EdgeRouter receives Data, Data = " << contentObject->getName()<<"; Data is signed by = " << contentObject->getSignature().getValue()<<std::endl;
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
EdgeRouter2::OnInterest(std::shared_ptr<const ndn::Interest> interest)
{
  ndn::App::OnInterest(interest);
  std::cout << "EdgeRouter receives Interest, Interest = " << interest->getName()<< std::endl;

  NS_LOG_DEBUG("Received Interest packet for " << interest->getName());

 
  if(interest->getName().getPrefix(1)!="/localhost")
  {

    uint32_t seq = interest->getName().at(-1).toSequenceNumber();
    std::cout<<"Interest SequenceNumber = "<<seq<<std::endl;

    Simulator::Schedule(Seconds(0), &EdgeRouter2::SendInterest, this);
  }

}

// Callback that will be called when Data arrives
void
EdgeRouter2::OnData(std::shared_ptr<const ndn::Data> data)
 {
  if (!m_active)
    return;

  App::OnData(data); // tracing inside

  NS_LOG_FUNCTION(this << data);
  NS_LOG_DEBUG("Receiving Data packet for " << data->getName());

  EdgeRouter2::SendData(data);

  }
} // namespace ndn
} // namespace ns3
