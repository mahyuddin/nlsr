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

// nlsr-lsu.h

#ifndef NLSR_LSU_H
#define NLSR_LSU_H

#include "ns3/ptr.h"
#include "ns3/header.h"
#include "ns3/ndn-data.h"

namespace ns3 {
namespace nlsr {

// LSU Name: /nlsr/<router>/<lsu>/<seq#>
//   lsu: a number/string
//   Seq#: timestamp (ms, unix_time) + #
//         timestamp is against replay attack
//
// All in TLV format
// LSU content:
//   lifetime: uint_32 in sec, count-down from N to 0
//             N = 1day default,  won’t be changed during propagation
//             soft-state: when timeout, remove the LSU from LSDB; the originator needs to refresh LSU before timeout
//   adjacency (optional): <routername, metric>*
//                         metric: numeric-value TLV (uint_32)
//   reachability of prefix (optional): <prefix, metric>*
//                                      prefix: name prefix

class LsuContent : public Header {

public:
  struct NeighborTuple
  {
    std::string routerName;
    uint16_t metric;
  };

  struct PrefixTuple
  {
    std::string prefixName;
    uint16_t metric;
  };

  LsuContent ();
  virtual ~LsuContent ();  

  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;
  void Print (std::ostream &os) const;
  uint32_t GetSerializedSize (void) const;
  void Serialize (Buffer::Iterator start) const;
  uint32_t Deserialize (Buffer::Iterator start);
  uint32_t Deserialize (Buffer::Iterator start, uint32_t messageSize);

  uint32_t GetLifetime () const; 
  void SetLifetime (uint32_t lifetime); 

  const std::vector<NeighborTuple> & GetAdjacency () const;
  void AddAdjacency (const std::string &routerName, uint16_t metric);

  const std::vector<PrefixTuple> & GetReachability () const;
  void AddReachability (const std::string &prefixName, uint16_t metric);

private:
  uint16_t GetAdjacencySize (void) const;
  uint16_t GetReachabilitySize (void) const;

private:
  uint32_t m_lifetime; // count-down timer for soft-state protocol 
  std::vector<NeighborTuple> m_adjacency;
  std::vector<PrefixTuple> m_reachability;

}; // class LsuContent
  
} // namespace nlsr
} // namespace ns3

#endif /* NLSR_LSU_H */