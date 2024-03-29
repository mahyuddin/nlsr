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

#ifndef SYNC_APP_H_
#define SYNC_APP_H_

#include "nlsr-lsu.h"
#include "sync-state.h"
#include "ns3/ndn-app.h"

namespace ns3 {
namespace ndn {

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

static const std::string SYNC_PREFIX = "/ndn/sync";
static const uint16_t SYNC_PREFIX_SIZE = 2;
static const double PACKET_LOSS_RATE = 0.1;

class SyncApp : public ndn::App, SyncState
{

public:
  SyncApp ();

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
  SendSyncInterest (uint64_t oldDigest, uint64_t newDigest);

  void
  SendSyncData (Ptr<ndn::Data> data);

  void
  SendUpdateInbetween (uint64_t digest1, uint64_t digest2);

  void
  PeriodicalSyncInterest ();

  const Ptr<ndn::Interest>
  BuildSyncInterest (uint64_t digest1, uint64_t digest2);

  Ptr<ndn::Name> 
  MakeSyncName (uint64_t oldDigest, uint64_t newDigest) const;

  uint64_t
  GetDigestFromName (Ptr<const ndn::Name> name, uint64_t & digest1, uint64_t & digest2) const;

  void
  GenerateNewUpdate ();

  void
  OnNewUpdate ();

  void
  SetOutstandingDigest (uint64_t digest);

  uint64_t
  GetOutstandingDigest () const;

  const std::string &
  GetRouterName () const;

  void
  SetRouterName (const std::string & routerName);

  uint64_t
  GetNextSequenceNumber ();

  bool
  IsPacketDropped () const;

  void
  SetUnknownDigest (uint64_t digest);

  uint64_t
  GetUnknownDigest () const;

private:
  std::string m_routerName;
  uint64_t m_seq;
  uint64_t m_outstandingDigest;
  uint64_t m_unknownDigest;

};

} // namespace nlsr
} // namespace ns3

#endif // SYNC_APP_H_
