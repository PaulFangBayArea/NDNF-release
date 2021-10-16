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


#include "NFVAPP2.hpp"
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

NS_LOG_COMPONENT_DEFINE("NFVAPP2");

namespace ns3 {
  namespace ndn {
NS_OBJECT_ENSURE_REGISTERED(NFVAPP2);


// register NS-3 type
TypeId
NFVAPP2::GetTypeId()
{
  static TypeId tid = TypeId("ns3::ndn::NFVAPP2").SetParent<App>().AddConstructor<NFVAPP2>()
      .AddAttribute("Prefix1", "Prefix, for which NFV node  handle the data", StringValue("/"),
                    MakeNameAccessor(&NFVAPP2::m_prefix1), MakeNameChecker())
      .AddAttribute("Prefix2", "Prefix, for which NFV node can execute the corrresponding NFV", StringValue("/"),
                    MakeNameAccessor(&NFVAPP2::m_prefix2), MakeNameChecker())
      .AddAttribute("Freshness", "Freshness of data packets, if 0, then unlimited freshness",
                    TimeValue(Seconds(0)), MakeTimeAccessor(&NFVAPP2::m_freshness),
                    MakeTimeChecker())
      .AddAttribute(
         "Signature",
         "Fake signature, 0 valid signature (default), other values application-specific",
         UintegerValue(0), MakeUintegerAccessor(&NFVAPP2::m_signature),
         MakeUintegerChecker<uint32_t>())
      .AddAttribute("KeyLocator",
                    "Name to be used for key locator.  If root, then key locator is not used",
                    NameValue(), MakeNameAccessor(&NFVAPP2::m_keyLocator), MakeNameChecker());;
  return tid;
}

// Processing upon start of the application
void
NFVAPP2::StartApplication()
{
  // initialize ndn::App
  ndn::App::StartApplication();
 
  // Add entry to FIB 
  FibHelper::AddRoute(GetNode(), m_prefix1, m_face, 0);

}

// Processing when application is stopped
void
NFVAPP2::StopApplication()
{
  // cleanup ndn::App
  ndn::App::StopApplication();
}

void
NFVAPP2::SendInterest()
{
  /////////////////////////////////////
  // Sending one Interest packet out //
  /////////////////////////////////////
 
  auto& nodePit = GetNode()->GetObject<ndn::L3Protocol>()->getForwarder()->getPit();
  nfd::Pit::const_iterator iter;
  
  //loop the FIB to find the Interest starts with /f
  for (iter = nodePit.begin(); iter != nodePit.end(); iter++) {
  
  if(iter->getName().getPrefix(1)==m_prefix2){
  std::cout<<"NFV node "<<GetNode()->GetId()<<" fetches PIT entry, PIT entry = " << iter->getName()<<std::endl;
  
  shared_ptr<Name> nameWithSequence = make_shared<Name>(iter->getName().getSubName(1,4));
  shared_ptr<Interest> interest = make_shared<Interest>();

  interest->setName(*nameWithSequence);
 
  Ptr<UniformRandomVariable> rand = CreateObject<UniformRandomVariable>();
  interest->setNonce(rand->GetValue(0, std::numeric_limits<uint32_t>::max()));
  interest->setCanBePrefix(false);
  interest->setInterestLifetime(ndn::time::seconds(1));
  
  NS_LOG_DEBUG("Sending Interest packet for " << *interest);
  std::cout<<"NFV node "<<GetNode()->GetId()<<" sends Interest, Interest = " << interest->getName()<<std::endl;
  
  m_transmittedInterests(interest, this, m_face);

  m_appLink->onReceiveInterest(*interest);
  }
}
}

void
NFVAPP2::SendData(std::shared_ptr<const ndn::Data> contentObject)
{ 
  
  
   auto& nodePit = GetNode()->GetObject<ndn::L3Protocol>()->getForwarder()->getPit();
   nfd::Pit::const_iterator iter;
   iter=nodePit.begin();
   
  for (iter = nodePit.begin(); iter != nodePit.end(); iter++) {
  
  if(iter->getName().getSubName(1,2)==contentObject->getName().getPrefix(2))
  {

  auto data = std::make_shared<ndn::Data>();
  data->setName(iter->getName());
  std::cout<<"NFV node "<<GetNode()->GetId()<<" executes function f on Data" <<std::endl;

  data->setFreshnessPeriod(::ndn::time::milliseconds(m_freshness.GetMilliSeconds()));
  
  
  //pass the content to the created Data
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
  std::cout<<"NFV node "<<GetNode()->GetId()<<" sends Data, Data = " << data->getName()<<std::endl;

  // to create real wire encoding
  data->wireEncode();

  
  m_transmittedDatas(data, this, m_face);
  m_appLink->onReceiveData(*data);
  }
  }
}

// Callback that will be called when Interest arrives
void
NFVAPP2::OnInterest(std::shared_ptr<const ndn::Interest> interest)
{
  ndn::App::OnInterest(interest);
  std::cout <<"NFV node "<<GetNode()->GetId()<<" receives Interest, Interest = " << interest->getName()<< std::endl;

  NS_LOG_DEBUG("Received Interest packet for " << interest->getName());
  
  //if Interest starts with /f
  if(interest->getName().getPrefix(1)==m_prefix2)
  {

    uint32_t seq = interest->getName().at(-1).toSequenceNumber();
    std::cout<<"Interest SequenceNumber = "<<seq<<std::endl;

    Simulator::Schedule(Seconds(0), &NFVAPP2::SendInterest, this);
  }
 
}

// Callback that will be called when Data arrives
void
NFVAPP2::OnData(std::shared_ptr<const ndn::Data> data)
{
  if (!m_active)
    return;

  App::OnData(data); // tracing inside
  NS_LOG_FUNCTION(this << data);
  NS_LOG_DEBUG("Receiving Data packet for " << data->getName());

  std::cout <<"NFV node "<<GetNode()->GetId()<<" receives Data, Data = " << data->getName()<<"; Data is signed by = " << data->getSignature().getValue()<<std::endl;
  NFVAPP2::SendData(data);

}
  } //namespace ndn
} // namespace ns3
