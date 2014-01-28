/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2011-2012 University of California, Los Angeles
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope tha t it will be useful,
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

// nlsr-protocol.h

#ifndef NLSR_PROTOCOL_H
#define NLSR_PROTOCOL_H

#include "nlsr-state.h"
#include "ns3/header.h"
#include "ns3/ptr.h"
#include "ns3/ndn-interest.h"
#include "ns3/ndn-data.h"

namespace ns3 {
namespace nlsr {

/// ========== Class NlsrProtcol ============

class NlsrProtocol : public LocalProtocolState {
  
public:
  NlsrProtocol ();
  
  virtual
  ~NlsrProtocol ();  

  static TypeId
  GetTypeId (void);

  virtual TypeId
  GetInstanceTypeId (void) const;

  static const Ptr<ndn::Interest>
  BuildSyncInterestWithDigest (uint64_t digest);

  Ptr<ndn::Data>
  ProcessSyncInterest (Ptr<const ndn::Interest> syncInterest);

  void
  ProcessSyncData (Ptr<const ndn::Data> syncData);

};

} // namespace nlsr
} // namespace ns3

#endif /* NLSR_PROTOCOL_H */