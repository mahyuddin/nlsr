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

#include "nlsr-app.h"
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

NS_LOG_COMPONENT_DEFINE ("NlsrApp");

namespace ns3 {
namespace nlsr {

NS_OBJECT_ENSURE_REGISTERED (NlsrApp);


NlsrApp::NlsrApp ()
{
  m_seq = 1;
  m_outstandingDigest = 0;
}

// register NS-3 type
TypeId
NlsrApp::GetTypeId ()
{
  static TypeId tid = TypeId ("NlsrApp")
    .SetParent<ndn::App> ()
    .AddConstructor<NlsrApp> ()
    ;
  return tid;
}

// Processing upon start of the application
void
NlsrApp::StartApplication ()
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

  Simulator::Schedule (Seconds (0.0), &NlsrApp::GenerateNewUpdate, this);
  Simulator::Schedule (Seconds (0.1), &NlsrApp::PeriodicalSyncInterest, this);
}

// Processing when application is stopped
void
NlsrApp::StopApplication ()
{
  // cleanup ndn::App
  ndn::App::StopApplication ();
}

void
NlsrApp::SendSyncInterest (uint64_t digest1, uint64_t digest2)
{
  const Ptr<ndn::Interest> interest = BuildSyncInterest (digest1, digest2);

  NS_LOG_DEBUG ("Sending Sync Interest: " << interest->GetName ());
  
  // Forward packet to lower (network) layer
  Simulator::ScheduleNow (&ndn::Face::ReceiveInterest, m_face, interest);

  // Call trace (for logging purposes)
  m_transmittedInterests (interest, this, m_face);
}

void
NlsrApp::PeriodicalSyncInterest ()
{
  const Ptr<ndn::Interest> interest = BuildSyncInterest (GetCurrentDigest (), 0);

  NS_LOG_DEBUG ("Sending Periodical Sync Interest: " << interest->GetName ());
  
  // Forward packet to lower (network) layer
  Simulator::ScheduleNow (&ndn::Face::ReceiveInterest, m_face, interest);

  // Call trace (for logging purposes)
  m_transmittedInterests (interest, this, m_face);

  Simulator::Schedule (Seconds (5), &NlsrApp::PeriodicalSyncInterest, this);
}

void
NlsrApp::SendSyncData (Ptr<ndn::Data> data)
{
  NS_LOG_DEBUG ("Sending Data packet for " << data->GetName ());

  // Forward packet to lower (network) layer
  Simulator::ScheduleNow (&ndn::Face::ReceiveData, m_face, data);

  // Call trace (for logging purposes)
  m_transmittedDatas (data, this, m_face);
}

void
NlsrApp::SendUpdateSinceThen (uint64_t digest)
{
  Ptr<LsuNameList> lsuNameList = Create<LsuNameList> ();
  GetUpdateSinceThen (digest, lsuNameList->Get ());

  Ptr<Packet> packet = Create<Packet> ();
  packet->AddHeader (*lsuNameList);

  Ptr<ndn::Data> data = Create<ndn::Data> (packet);
  data->SetName (MakeSyncName (digest, GetCurrentDigest ()));

  SendSyncData (data);
}

void
NlsrApp::SendUpdateByThen (uint64_t digest)
{
  Ptr<LsuNameList> lsuNameList = Create<LsuNameList> ();
  GetUpdateByThen (digest, lsuNameList->Get ());

  Ptr<Packet> packet = Create<Packet> ();
  packet->AddHeader (*lsuNameList);

  Ptr<ndn::Data> data = Create<ndn::Data> (packet);
  data->SetName (MakeSyncName (INITIAL_DIGEST, digest));

  SendSyncData (data);
}

// Callback that will be called when Interest arrives
void
NlsrApp::OnInterest (Ptr<const ndn::Interest> interest)
{
  NS_LOG_DEBUG ("Receive Interest packet for " << interest->GetName ());

  if ( IsPacketDropped () ) {
    NS_LOG_DEBUG ("Packet lost !");
    return;
  }

  Ptr<const ndn::Name> name = interest->GetNamePtr ();
  uint64_t digest1 = 0;
  uint64_t digest2 = 0;
  GetDigestFromName (name, digest1, digest2);

  if (digest2 == 0) {
    NS_LOG_DEBUG ("Sync Request! " << digest1);
    if (IsCurrentDigest (digest1)) {
      NS_LOG_DEBUG ("============= Synced! ============" << digest1);
      SetOutstandingDigest (digest1);
    } else {
      if (IsDigestInLog (digest1)) {
        NS_LOG_DEBUG ("Is In Log! " << digest1);
        SendUpdateSinceThen (digest1);
      } else {
        NS_LOG_DEBUG ("============= Not In Log! ============" << digest1);
        SendSyncInterest (INITIAL_DIGEST, digest1);
      }
    }
  } else {
    NS_LOG_DEBUG ("Resync Request! " << digest1 << " " << digest2);
    if (IsDigestInLog (digest2)) {
        NS_LOG_DEBUG ("=========== Resync with Digest: " << digest2);
        //SendUpdateSinceThen (0);
        SendUpdateByThen (digest2);
    } else {
        NS_LOG_DEBUG ("=========== Cannot Resync with Digest: " << digest2);
    } 
  }
}

// Callback that will be called when Data arrives
void
NlsrApp::OnData (Ptr<const ndn::Data> data)
{ 

  NS_LOG_DEBUG ("Receiving Data packet for " << data->GetName ());

  if ( IsPacketDropped () ) {
    NS_LOG_DEBUG ("Packet loss !");
    return;
  }

  Ptr<Packet> payload = data->GetPayload ()->Copy ();  
  
  //std::cout << "Content Size is " << payload->GetSize () << std::endl;

  nlsr::LsuNameList nameList;
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
NlsrApp::GenerateNewUpdate ()
{
  std::string s;
  std::string old;
  IdSeqToName ("/" + GetRouterName () + "/lsu1" , GetNextSequenceNumber (), s);
  Update (s, old);
  NS_LOG_DEBUG ("New Updates: " << s << " New Digest: " << GetCurrentDigest ());

  IdSeqToName ("/" + GetRouterName () + "/lsu2", GetNextSequenceNumber (), s);
  Update (s, old);
  NS_LOG_DEBUG ("New Updates: " << s << " New Digest: " << GetCurrentDigest ()); 

  OnNewUpdate ();
  UniformVariable rand (1, 2);
  Simulator::Schedule (Seconds (rand.GetValue ()), &NlsrApp::GenerateNewUpdate, this);
}

void
NlsrApp::OnNewUpdate ()
{
  if (GetOutstandingDigest () == 0) {
    NS_LOG_DEBUG ("No Outstanding Interest");
  } else {
    SendUpdateSinceThen (GetOutstandingDigest ());
    SetOutstandingDigest (0);
  }
  SendSyncInterest (GetCurrentDigest(), 0);
}

const Ptr<ndn::Interest>
NlsrApp::BuildSyncInterest (uint64_t digest1, uint64_t digest2)
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

  return interest;
}

void
NlsrApp::SetOutstandingDigest (uint64_t digest)
{
  m_outstandingDigest = digest;
}

uint64_t
NlsrApp::GetOutstandingDigest () const
{
  return m_outstandingDigest;
}

uint64_t
NlsrApp::GetNextSequenceNumber ()
{
  return m_seq++;
}

bool
NlsrApp::IsPacketDropped () const
{
  UniformVariable rand (0, 1);
  double prob = 0;
  return rand.GetValue () >= prob ? false : true;
}

Ptr<ndn::Name> 
NlsrApp::MakeSyncName (uint64_t oldDigest, uint64_t newDigest) const
{
  Ptr<ndn::Name> prefix = Create<ndn::Name> (SYNC_PREFIX);

  prefix->appendNumber (oldDigest);

  if (newDigest != 0) {
    prefix->appendNumber (newDigest);
  }
  return prefix;
}

void
NlsrApp::GetDigestFromName (Ptr<const ndn::Name> name, uint64_t & digest1, uint64_t & digest2) const
{ 
  NS_ASSERT (name->getPrefix (2).toUri ().compare (SYNC_PREFIX) == 0);

  digest1 = name->get (2).toNumber ();  
  if (name->size () == 4) {
    digest2 = name->get (3).toNumber ();
  } else {
    digest2 = 0;
  }
}

const std::string &
NlsrApp::GetRouterName () const
{
  return m_routerName;
}

void
NlsrApp::SetRouterName (const std::string & routerName)
{
  m_routerName = routerName;
}

} // namespace nlsr
} // namespace ns3
