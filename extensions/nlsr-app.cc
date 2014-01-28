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
#include "nlsr-protocol.h"
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
  Ptr<ndn::Name> prefix = Create<ndn::Name> ("/nlsr"); // now prefix contains ``/``

  /////////////////////////////////////////////////////////////////////////////
  // Creating FIB entry that ensures that we will receive incoming Interests //
  /////////////////////////////////////////////////////////////////////////////

  // Get FIB object
  Ptr<ndn::Fib> fib = GetNode ()->GetObject<ndn::Fib> ();

  // Add entry to FIB
  // Note that ``m_face`` is cretaed by ndn::App
  Ptr<ndn::fib::Entry> fibEntry = fib->Add (*prefix, m_face, 0);

  Simulator::Schedule (Seconds (0.0), &NlsrApp::SendInterest, this);
}

// Processing when application is stopped
void
NlsrApp::StopApplication ()
{
  // cleanup ndn::App
  ndn::App::StopApplication ();
}

void
NlsrApp::SendInterest ()
{
  //const Ptr<ndn::Interest> interest = nlsr::NlsrProtocol::BuildSyncInterestWithDigest (m_nlsr.GetCurrentDigest ());
  const Ptr<ndn::Interest> interest = nlsr::NlsrProtocol::BuildSyncInterestWithDigest (ns3::Hash64 ("/nlsr/resync"));

  //NS_LOG_DEBUG ("Sending Sync Interest with digest " << ns3::Hash64("123"));
  
  // Forward packet to lower (network) layer
  Simulator::ScheduleNow (&ndn::Face::ReceiveInterest, m_face, interest);

  // Call trace (for logging purposes)
  m_transmittedInterests (interest, this, m_face);
}

// Callback that will be called when Interest arrives
void
NlsrApp::OnInterest (Ptr<const ndn::Interest> interest)
{
  NS_LOG_DEBUG ("Received Interest packet for " << interest->GetName ());

  // Create and configure ndn::ContentObjectHeader and ndn::ContentObjectTail
  // (header is added in front of the packet, tail is added at the end of the packet)

  // Note that Interests send out by the app will not be sent back to the app !

  Ptr<ndn::Data> data = m_nlsr.ProcessSyncInterest (interest);

  // Ptr<nlsr::LsuNameList> nameList = Create<nlsr::LsuNameList> ();
  // nameList->AddName ("/nlsr/router1/1234");
  // nameList->AddName ("/nlsr/router2/1234");

  // Ptr<Packet> packet = Create<Packet> ();
  // packet->AddHeader (*nameList);

  
  // Ptr<ndn::Data> data = Create<ndn::Data> (packet);
  // data->SetName (Create<ndn::NameComponents> (interest->GetName ())); // data will have the same name as Interests

  NS_LOG_DEBUG ("Sending ContentObject packet for " << data->GetName ());

  // Forward packet to lower (network) layer
  Simulator::ScheduleNow (&ndn::Face::ReceiveData, m_face, data);

  // Call trace (for logging purposes)
  m_transmittedDatas (data, this, m_face);
}

// Callback that will be called when Data arrives
void
NlsrApp::OnData (Ptr<const ndn::Data> data)
{
  NS_LOG_DEBUG ("Receiving Data packet for " << data->GetName ());

  // std::cout << "DATA received for name " << data->GetName () << std::endl;
  
  // Ptr<Packet> payload = data->GetPayload ()->Copy ();  
  
  // std::cout << "Content Size is " << payload->GetSize () << std::endl;

  // // nlsr::LsuContent lsu;
  // // payload->RemoveHeader (lsu);
  // // lsu.Print(std::cout);

  // nlsr::LsuNameList nameList;
  // payload->RemoveHeader (nameList);
  // nameList.Print(std::cout);

  m_nlsr.ProcessSyncData (data);

  // nlsr::HelloData helloData;
  // payload->RemoveHeader (helloData);
  // helloData.Print(std::cout);
}

} // namespace nlsr
} // namespace ns3
