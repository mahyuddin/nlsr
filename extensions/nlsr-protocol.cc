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

// nlsr-protocol.cc

#include "nlsr-protocol.h"
#include "ns3/assert.h"
#include "ns3/log.h"
#include "ns3/random-variable.h"


NS_LOG_COMPONENT_DEFINE ("NlsrProtocol");

namespace ns3 {
namespace nlsr {

NS_OBJECT_ENSURE_REGISTERED (NlsrProtocol);

NlsrProtocol::NlsrProtocol ()
{
}

NlsrProtocol::~NlsrProtocol ()
{
}

TypeId
NlsrProtocol::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::nlsr::NlsrProtocol");
  return tid;
}

TypeId
NlsrProtocol::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

const Ptr<ndn::Interest>
NlsrProtocol::BuildSyncInterestWithDigest (uint64_t digest)
{
  NS_LOG_DEBUG ("");

  Ptr<ndn::Name> prefix = Create<ndn::Name> ("/nlsr/sync");
  // append digest to prefix
  prefix->appendNumber (digest);

  // Create and configure ndn::InterestHeader
  Ptr<ndn::Interest> interest = Create<ndn::Interest> ();
  UniformVariable rand (0,std::numeric_limits<uint32_t>::max ());
  interest->SetNonce            (rand.GetValue ());
  interest->SetName             (prefix);
  interest->SetInterestLifetime (Seconds (1.0));

  return interest;
}

uint64_t
NlsrProtocol::GetDigestFromSyncInterest (Ptr<const ndn::Interest> syncInterest)
{

  Ptr<const ndn::Name> name = syncInterest->GetNamePtr ();
  
  NS_ASSERT (name->size () == 3);
  NS_ASSERT (name->getPrefix (2).toUri ().compare ("/nlsr/sync") == 0);

  //for (int i = 0; i < name->size (); i++ )
  //{
  //   NS_LOG_DEBUG ("Component " << i << " :" << name->get (i).toUri ());
  //}

  return name->get (2).toNumber (); 
}


} // namespace nlsr
} // namespace ns3