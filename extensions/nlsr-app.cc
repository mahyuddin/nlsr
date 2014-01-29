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
NlsrApp::SendSyncInterest ()
{
  uint64_t digest = GetCurrentDigest ();
  const Ptr<ndn::Interest> interest = BuildSyncInterestWithDigest (digest, true);

  NS_LOG_DEBUG ("Sending Sync Interest with digest: " << digest);
  
  // Forward packet to lower (network) layer
  Simulator::ScheduleNow (&ndn::Face::ReceiveInterest, m_face, interest);

  // Call trace (for logging purposes)
  m_transmittedInterests (interest, this, m_face);

  //Simulator::Schedule (Seconds (5), &NlsrApp::SendSyncInterest, this);

}

void
NlsrApp::SendResyncInterest (uint64_t digest)
{
  const Ptr<ndn::Interest> interest = BuildSyncInterestWithDigest (digest, false);

  NS_LOG_DEBUG ("Sending Resync Interest: " << interest->GetName ());
  
  // Forward packet to lower (network) layer
  Simulator::ScheduleNow (&ndn::Face::ReceiveInterest, m_face, interest);

  // Call trace (for logging purposes)
  m_transmittedInterests (interest, this, m_face);
}

void
NlsrApp::PeriodicalSyncInterest ()
{
  uint64_t digest = GetCurrentDigest ();
  const Ptr<ndn::Interest> interest = BuildSyncInterestWithDigest (digest, true);

  NS_LOG_DEBUG ("Sending PeriodicalSyncInterest with digest: " << digest);
  
  // Forward packet to lower (network) layer
  Simulator::ScheduleNow (&ndn::Face::ReceiveInterest, m_face, interest);

  // Call trace (for logging purposes)
  m_transmittedInterests (interest, this, m_face);

  Simulator::Schedule (Seconds (5), &NlsrApp::PeriodicalSyncInterest, this);
}

void
NlsrApp::SendSyncData (Ptr<ndn::Data> data)
{
  NS_LOG_DEBUG ("Sending ContentObject packet for " << data->GetName ());

  // Forward packet to lower (network) layer
  Simulator::ScheduleNow (&ndn::Face::ReceiveData, m_face, data);

  // Call trace (for logging purposes)
  m_transmittedDatas (data, this, m_face);
}

// void
// NlsrApp::WaitForUpdates (uint64_t digest)
// {
//   if (IsCurrentDigest (digest)) {
//       NS_LOG_DEBUG ("No Update after Waiting: " << digest);
//       //Simulator::Schedule (Seconds (3), &NlsrApp::WaitForUpdates, this, digest);
//       return;
//   } else {
//     if (IsDigestInLog (digest)) {
//       NS_LOG_DEBUG ("Is In Log! " << digest);
//       SendUpdateSinceThen (digest);
//     } else {
//       NS_LOG_DEBUG ("Not In Log! " << digest);
//       //SendSyncInterest (GetCurrentDigest ());
//       SendResyncInterest (digest);
//       return;
//     }
//   }  
// }

void
NlsrApp::SendUpdateSinceThen (uint64_t digest)
{
  Ptr<Packet> packet = Create<Packet> ();
  Ptr<LsuNameList> lsuNameList = Create<LsuNameList> ();
  Ptr<ndn::Name> prefix = Create<ndn::Name> ("/nlsr");

  if (digest == 0) {
    GetAllLsuName (lsuNameList);
    digest = GetCurrentDigest ();
    prefix->append ("resync");
  } else {
    GetUpdateSinceThen (digest, lsuNameList);    
    prefix->append ("sync");
  }

  Ptr<ndn::Data> data = Create<ndn::Data> (packet);
  prefix->appendNumber (digest);
  data->SetName (prefix);
  packet->AddHeader (*lsuNameList);
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
  
  NS_ASSERT (name->size () == 3);
  if (name->getPrefix (2).toUri ().compare ("/nlsr/sync") == 0) {
    uint64_t digest = name->get (2).toNumber ();
    NS_LOG_DEBUG ("Sync Request! " << digest);
    if (IsCurrentDigest (digest)) {
      NS_LOG_DEBUG ("============= Synced! ============" << digest);
      //Simulator::Schedule (Seconds (3), &NlsrApp::WaitForUpdates, this, digest);
      SetOutstandingDigest (digest);
    } else {
      if (IsDigestInLog (digest)) {
        NS_LOG_DEBUG ("Is In Log! " << digest);
        SendUpdateSinceThen (digest);
      } else {
        NS_LOG_DEBUG ("============= Not In Log! ============" << digest);
        SendResyncInterest (digest);
      }
    }
  } else {
    if (name->getPrefix (2).toUri ().compare ("/nlsr/resync") == 0) {
      uint64_t digest = name->get (2).toNumber ();
      NS_LOG_DEBUG ("Resync Request! " << digest);
      if (IsCurrentDigest (digest)) {
          NS_LOG_DEBUG ("Resync with CurrenetDigest: " << digest);
          SendUpdateSinceThen (0);
      } else {
          NS_LOG_DEBUG ("Cannot Resync with CurrenetDigest: " << GetCurrentDigest ());
      }
    } else {
      NS_LOG_DEBUG ("Unknown Request! " << name);
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

  ProcessSyncData (data);

  //Simulator::Schedule (Seconds (0.1), &NlsrApp::SendSyncInterest, this);
}

void
NlsrApp::ProcessSyncData (Ptr<const ndn::Data> syncData)
{
  NS_LOG_DEBUG ("Receiving Data packet for " << syncData->GetName ());
  
  Ptr<Packet> payload = syncData->GetPayload ()->Copy ();  
  
  //std::cout << "Content Size is " << payload->GetSize () << std::endl;

  nlsr::LsuNameList nameList;
  payload->RemoveHeader (nameList);
  nameList.Print(std::cout);
  std::vector<std::string> outLsuNameList;

  bool isNew = NewerLsuNameFilter (nameList.GetNameList (), outLsuNameList);

  for (std::vector<std::string>::const_iterator i = outLsuNameList.begin ();
       i != outLsuNameList.end ();
       i++) 
  {
    InsertNewLsu (*i, 0);
  }

  OnNewUpdate ();
}

void
NlsrApp::GenerateNewUpdate ()
{
  std::string s;
  LsuIdSeqToName ("/" + GetRouterName () + "/lsu1" , GetNextSequenceNumber (), s);
  InsertNewLsu (s, 0);
  NS_LOG_DEBUG ("New Updates: " << s << " New Digest: " << GetCurrentDigest ());

  LsuIdSeqToName ("/" + GetRouterName () + "/lsu2", GetNextSequenceNumber (), s);
  InsertNewLsu (s, 0);
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
  SendSyncInterest ();
}

const Ptr<ndn::Interest>
NlsrApp::BuildSyncInterestWithDigest (uint64_t digest, bool isSync)
{
  //NS_LOG_DEBUG ("");
  std::string prefix = (isSync == true?  "/nlsr/sync" : "/nlsr/resync");
  Ptr<ndn::Name> name = Create<ndn::Name> (prefix);
  name->appendNumber (digest);

  // Create and configure ndn::InterestHeader
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
  double prob = 0.1;
  return rand.GetValue () > prob ? false : true;
}

} // namespace nlsr
} // namespace ns3
