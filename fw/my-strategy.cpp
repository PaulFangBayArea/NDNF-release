/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014-2018,  Regents of the University of California,
 *                           Arizona Board of Regents,
 *                           Colorado State University,
 *                           University Pierre & Marie Curie, Sorbonne University,
 *                           Washington University in St. Louis,
 *                           Beijing Institute of Technology,
 *                           The University of Memphis.
 *
 * This file is part of NFD (Named Data Networking Forwarding Daemon).
 * See AUTHORS.md for complete list of NFD authors and contributors.
 *
 * NFD is free software: you can redistribute it and/or modify it under the terms
 * of the GNU General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 *
 * NFD is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * NFD, e.g., in COPYING.md file.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "my-strategy.hpp"
#include "algorithm.hpp"
#include "common/logger.hpp"

namespace nfd {
namespace fw {

NFD_REGISTER_STRATEGY(MyStrategy);

NFD_LOG_INIT(MyStrategy);

const time::milliseconds MyStrategy::RETX_SUPPRESSION_INITIAL(10);
const time::milliseconds MyStrategy::RETX_SUPPRESSION_MAX(250);

MyStrategy::MyStrategy(Forwarder& forwarder, const Name& name)
  : Strategy(forwarder)
  , ProcessNackTraits(this)
  , m_retxSuppression(RETX_SUPPRESSION_INITIAL,
                      RetxSuppressionExponential::DEFAULT_MULTIPLIER,
                      RETX_SUPPRESSION_MAX)
{
  ParsedInstanceName parsed = parseInstanceName(name);
  if (!parsed.parameters.empty()) {
    BOOST_THROW_EXCEPTION(std::invalid_argument("MyStrategy does not accept parameters"));
  }
  if (parsed.version && *parsed.version != getStrategyName()[-1].toVersion()) {
    BOOST_THROW_EXCEPTION(std::invalid_argument(
      "MyStrategy does not support version " + to_string(*parsed.version)));
  }
  this->setInstanceName(makeInstanceName(name, getStrategyName()));
}

const Name&
MyStrategy::getStrategyName()
{
  //std::cout << "mystrategy" << std::endl;
  static Name strategyName("/localhost/nfd/strategy/my/%FD%10");
  return strategyName;
}

void
 MyStrategy::afterReceiveInterest(const FaceEndpoint& ingress, const Interest& interest,
                       const shared_ptr<pit::Entry>& pitEntry)
{
  std::cout << "--- An interest is received ---" << std::endl;
  

  const fib::Entry& fibEntry = this->lookupFib(*pitEntry);
  const fib::NextHopList& nexthops = fibEntry.getNextHops();

  for (const auto& nexthop : nexthops)
  {
    Face& outFace = nexthop.getFace();
    this->sendInterest(pitEntry, FaceEndpoint(outFace, 0), interest);
    NFD_LOG_DEBUG(interest << " from=" << ingress << " pitEntry-to=" << outFace.getId());
    std::cout<<interest << " from=" << ingress << " pitEntry-to=" << outFace.getId()<<std::endl;
  }
}

void
MyStrategy::afterReceiveNack(const FaceEndpoint& ingress, const lp::Nack& nack,
                   const shared_ptr<pit::Entry>& pitEntry)

{

  this->processNack(ingress.face, nack, pitEntry);
}

} // namespace fw
} // namespace nfd

