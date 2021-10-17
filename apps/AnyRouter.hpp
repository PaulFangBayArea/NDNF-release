//Author: Puming Fang
//Network Systems Lab, UMass Amherst


#ifndef EdgeRouter2_H
#define EdgeRouter2_H

#include "ns3/ndnSIM/model/ndn-common.hpp"

#include "ndn-app.hpp"
#include "ns3/ndnSIM/model/ndn-common.hpp"

#include "ns3/nstime.h"
#include "ns3/ptr.h"

namespace ns3 {
  namespace ndn {

/**
 * @brief A simple custom application
 *
 * This applications demonstrates how to send Interests and respond with Datas to incoming interests
 *
 * When application starts it "sets interest filter" (install FIB entry) for /prefix/sub, as well as
 * sends Interest for this prefix
 *
 * When an Interest is received, it is replied with a Data with 1024-byte fake payload
 */
class AnyRouter : public ndn::App {
public:
  // register NS-3 type "CustomApp"
  static TypeId
  GetTypeId();
  AnyRouter();
  // (overridden from ndn::App) Processing upon start of the application
  virtual void
  StartApplication();

  // (overridden from ndn::App) Processing when application is stopped
  virtual void
  StopApplication();

  // (overridden from ndn::App) Callback that will be called when Interest arrives
  virtual void
  OnInterest(std::shared_ptr<const ndn::Interest> interest);

  // (overridden from ndn::App) Callback that will be called when Data arrives
    // (overridden from ndn::App) Callback that will be called when Data arrives

  // (overridden from ndn::App) Callback that will be called when Data arrives
  virtual void
  OnData(std::shared_ptr<const ndn::Data> contentObject);


  
private:
  void
  SendInterest();
  
private:
  void
  SendData(std::shared_ptr<const ndn::Data> contentObject);

protected:
  //Ptr<UniformRandomVariable> m_rand; ///< @brief nonce generator
  Name m_prefix1;
  Name m_prefix2;

  uint32_t m_seq;      ///< @brief currently requested sequence number
  uint32_t m_seqMax;   ///< @brief maximum number of sequence number
  EventId m_sendEvent; ///< @brief EventId of pending "send packet" event
  Time m_retxTimer;    ///< @brief Currently estimated retransmission timer
  EventId m_retxEvent; ///< @brief Event to check whether or not retransmission should be performed

  //Ptr<RttEstimator> m_rtt; ///< @brief RTT estimator

  Time m_offTime;          ///< \brief Time interval between packets
  Name m_interestName;     ///< \brief NDN Name of the Interest (use Name)
  Time m_interestLifeTime; ///< \brief LifeTime for interest packet
  Time m_freshness;

  uint32_t m_signature;
  Name m_keyLocator;
  };
  }
} // namespace ns3

#endif // NFV_APP_H_
