/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2011-2012 University of California, Los Angeles
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Yu Zhang <yuzhang@hit.edu.cn>
 */

// nlsr-app.h

#ifndef NLSR_APP_H_
#define NLSR_APP_H_

#include "nlsr-protocol.h"
#include "ns3/ndn-app.h"

namespace ns3 {
namespace nlsr {

/**
 * @brief A simple custom application
 *
 * This applications demonstrates how to send Interests and respond with ContentObjects to incoming interests
 *
 * When application starts it "sets interest filter" (install FIB entry) for /prefix/sub, as well as
 * sends Interest for this prefix
 *
 * When an Interest is received, it is replied with a ContentObject with 1024-byte fake payload
 */
class NlsrApp : public ndn::App
{
public:
  // register NS-3 type "NlsrApp"
  static TypeId
  GetTypeId ();
  
  // (overridden from ndn::App) Processing upon start of the application
  virtual void
  StartApplication ();

  // (overridden from ndn::App) Processing when application is stopped
  virtual void
  StopApplication ();

  // (overridden from ndn::App) Callback that will be called when Interest arrives
  virtual void
  OnInterest (Ptr<const ndn::Interest> interest);

  // (overridden from ndn::App) Callback that will be called when Data arrives
  virtual void
  OnData (Ptr<const ndn::Data> data);

private:
  void
  SendInterest ();

private:
  nlsr::NlsrProtocol m_nlsr;

};

} // namespace nlsr
} // namespace ns3

#endif // NLSR_APP_H_
