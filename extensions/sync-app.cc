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

// nlsr-app.cc

#include "sync-app.h"
#include "ns3/ptr.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/packet.h"
#include "ns3/header.h"

#include "ns3/ndn-app-face.h"
#include "ns3/ndn-interest.h"
#include "ns3/ndn-data.h"

#include "ns3/ndn-fib.h"
#include "ns3/random-variable.h"

#include <sstream>

NS_LOG_COMPONENT_DEFINE ("SyncApp");

namespace ns3 {
namespace ndn {

NS_OBJECT_ENSURE_REGISTERED (SyncApp);


SyncApp::SyncApp ()
{
  m_seq = 1;
  m_outstandingDigest = 0;
}

// register NS-3 type
TypeId
SyncApp::GetTypeId ()
{
  static TypeId tid = TypeId ("SyncApp")
    .SetParent<ndn::App> ()
    .AddConstructor<SyncApp> ()
    ;
  return tid;
}

// Processing upon start of the application
void
SyncApp::StartApplication ()
{
  // initialize ndn::App
  ndn::App::StartApplication ();

  // Create a name components object for name ``/prefix/sub``
  Ptr<ndn::Name> prefix = Create<ndn::Name> ("/"); // now prefix contains ``/``

  // Get FIB object
  Ptr<ndn::Fib> fib = GetNode ()->GetObject<ndn::Fib> ();

  // Add entry to FIB
  // Note that ``m_face`` is cretaed by ndn::App
  Ptr<ndn::fib::Entry> fibEntry = fib->Add (*prefix, m_face, 0);

  std::stringstream ss;
  ss << GetNode ()-> GetId ();
  SetRouterName ("router-" +  ss.str());
  NS_LOG_DEBUG ("Starting ... Router: " << GetRouterName ());

  Simulator::Schedule (Seconds (0.0), &SyncApp::PeriodicalSyncInterest, this);

  Simulator::Schedule (Seconds (1), &SyncApp::GenerateNewUpdate, this);
}

// Processing when application is stopped
void
SyncApp::StopApplication ()
{
  // cleanup ndn::App
  ndn::App::StopApplication ();
}

// Callback that will be called when Interest arrives
void
SyncApp::OnInterest (Ptr<const ndn::Interest> interest)
{
  Ptr<const ndn::Name> name = interest->GetNamePtr ();
  uint64_t digest1 = 0;
  uint64_t digest2 = 0;
  GetDigestFromName (name, digest1, digest2);

  NS_LOG_DEBUG ("Receive Interest packet: " << digest1 << " " << digest2);

  if ( IsPacketDropped () ) { NS_LOG_DEBUG ("Interest Packet Lost !"); return; }

  if (digest2 == 0) {
    if (GetCurrentDigest () == digest1) {
      NS_LOG_DEBUG ("============= Synced! ============" << digest1);
      SetOutstandingDigest (digest1);
    } else {
      if (IsDigestInLog (digest1)) {
        NS_LOG_DEBUG ("============= Known! =============" << digest1);
        SendUpdateInbetween (digest1, GetCurrentDigest ());
      } else {
        NS_LOG_DEBUG ("============= Unknown! ============" << digest1);
        if (digest1 == GetUnknownDigest ()) { // This is a temporary solution, may lead to a deadlock
          SendSyncInterest (INITIAL_DIGEST, digest1);
        }
        SetUnknownDigest (digest1);
        SendSyncInterest (GetSyncDigest (), digest1);
      }
    }
  } else {
    if (IsDigestInLog (digest2)) {
        NS_LOG_DEBUG ("=========== Resynced! ============" << digest1 << " " << digest2);
        SendUpdateInbetween (digest1, digest2);
    } else {
        NS_LOG_DEBUG ("=========== Cannot Resync! ===========" << digest1 << " " << digest2);
    } 
  }
}

// Callback that will be called when Data arrives
void
SyncApp::OnData (Ptr<const ndn::Data> data)
{

  if ( IsPacketDropped () ) {
    NS_LOG_DEBUG ("Data Packet Lost!");
    return;
  }
  uint64_t digest1, digest2;
  GetDigestFromName (data->GetNamePtr (), digest1, digest2);

  NS_LOG_DEBUG ("Receiving Data packet: " << digest1 <<  " " << digest2);

  Ptr<Packet> payload = data->GetPayload ()->Copy ();    
  NameListHeader nameList;
  payload->RemoveHeader (nameList);
  nameList.Print(std::cout);
  std::vector<std::string> newNameList;
  bool hasNew = false;

  for (std::vector<std::string>::const_iterator i = nameList.GetNameList ().begin ();
       i != nameList.GetNameList ().end ();
       i++) 
  {
    std::string oldName;
    if (Update (*i, oldName) == true) {
      hasNew = true;
      newNameList.push_back (*i);
    }
  }

  if (hasNew) OnNewUpdate ();
}

void
SyncApp::SendSyncInterest (uint64_t digest1, uint64_t digest2)
{
  const Ptr<ndn::Interest> interest = BuildSyncInterest (digest1, digest2);

  NS_LOG_DEBUG ("Sending Sync Interest: " << digest1 << " " << digest2);
  
  // Forward packet to lower (network) layer
  Simulator::ScheduleNow (&ndn::Face::ReceiveInterest, m_face, interest);

  // Call trace (for logging purposes)
  m_transmittedInterests (interest, this, m_face);
}

void
SyncApp::PeriodicalSyncInterest ()
{
  SendSyncInterest (GetCurrentDigest (), 0);

  Simulator::Schedule (Seconds (3), &SyncApp::PeriodicalSyncInterest, this);
}

void
SyncApp::SendSyncData (Ptr<ndn::Data> data)
{
  //NS_LOG_DEBUG ("Sending Data packet for " << data->GetName ());

  // Forward packet to lower (network) layer
  Simulator::ScheduleNow (&ndn::Face::ReceiveData, m_face, data);

  // Call trace (for logging purposes)
  m_transmittedDatas (data, this, m_face);
}

void
SyncApp::SendUpdateInbetween (uint64_t digest1, uint64_t digest2)
{
  Ptr<NameListHeader> lsuNameList = Create<NameListHeader> ();
  if (GetUpdateInbetween (digest1, digest2, lsuNameList->Get ()) == false)
    return;

  Ptr<Packet> packet = Create<Packet> ();
  packet->AddHeader (*lsuNameList);

  Ptr<ndn::Data> data = Create<ndn::Data> (packet);
  data->SetName (MakeSyncName (digest1, digest2));
  NS_LOG_DEBUG ("Sending Data:" << digest1 << " " << digest2);

  SendSyncData (data);
}

/// ========================================

void
SyncApp::GenerateNewUpdate ()
{
  std::string s;
  std::string old;

  std::stringstream out;
  out << GetNextSequenceNumber ();

  IdSeqToName ("/" + GetRouterName () + "/unit-" + out.str(), 1, s);
  Update (s, old);
  NS_LOG_DEBUG ("New Updates: " << s << " New Digest: " << GetCurrentDigest ());

  OnNewUpdate ();
  UniformVariable rand (1, 2);
  Simulator::Schedule (Seconds (rand.GetValue ()), &SyncApp::GenerateNewUpdate, this);
}

void
SyncApp::OnNewUpdate ()
{
  if (GetOutstandingDigest () == 0) {
    NS_LOG_DEBUG ("No Outstanding Interest");
  } else {
    SendUpdateInbetween (GetOutstandingDigest (), GetCurrentDigest ());
    SetOutstandingDigest (0);
  }
  SendSyncInterest (GetCurrentDigest(), 0);
}

const Ptr<ndn::Interest>
SyncApp::BuildSyncInterest (uint64_t digest1, uint64_t digest2)
{
  Ptr<ndn::Name> name = Create<ndn::Name> (SYNC_PREFIX);
  name->appendNumber (digest1);
  if (digest2 != 0) {
    name->appendNumber (digest2);
  }
  Ptr<ndn::Interest> interest = Create<ndn::Interest> ();
  UniformVariable rand (0,std::numeric_limits<uint32_t>::max ());
  interest->SetNonce            (rand.GetValue ());
  interest->SetName             (name);
  interest->SetInterestLifetime (Seconds (5.0));
  interest->SetScope            (2);  

  return interest;
}

void
SyncApp::SetOutstandingDigest (uint64_t digest)
{
  m_outstandingDigest = digest;
  IncreaseCounter (digest);
}

uint64_t
SyncApp::GetOutstandingDigest () const
{
  return m_outstandingDigest;
}

uint64_t
SyncApp::GetNextSequenceNumber ()
{
  return m_seq++;
}

bool
SyncApp::IsPacketDropped () const
{
  UniformVariable rand (0, 1);
  double prob = PACKET_LOSS_RATE;
  return rand.GetValue () >= prob ? false : true;
}

Ptr<ndn::Name> 
SyncApp::MakeSyncName (uint64_t oldDigest, uint64_t newDigest) const
{
  Ptr<ndn::Name> prefix = Create<ndn::Name> (SYNC_PREFIX);

  prefix->appendNumber (oldDigest);

  if (newDigest != 0) {
    prefix->appendNumber (newDigest);
  }
  return prefix;
}

uint64_t
SyncApp::GetDigestFromName (Ptr<const ndn::Name> name, uint64_t & digest1, uint64_t & digest2) const
{ 
  NS_ASSERT (name->getPrefix (SYNC_PREFIX_SIZE).toUri ().compare (SYNC_PREFIX) == 0);

  digest1 = name->get (SYNC_PREFIX_SIZE).toNumber ();  
  if (name->size () == SYNC_PREFIX_SIZE + 2) {
    digest2 = name->get (SYNC_PREFIX_SIZE + 1).toNumber ();
  } else {
    digest2 = 0;
  }
  return digest1;
}

const std::string &
SyncApp::GetRouterName () const
{
  return m_routerName;
}

void
SyncApp::SetRouterName (const std::string & routerName)
{
  m_routerName = routerName;
}

uint64_t
SyncApp::GetUnknownDigest () const
{
  return m_unknownDigest;
}

void
SyncApp::SetUnknownDigest (uint64_t digest)
{
  m_unknownDigest = digest;
}

} // namespace ndn
} // namespace ns3
