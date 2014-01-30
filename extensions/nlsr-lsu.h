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

// nlsr-lsu.h

#ifndef NLSR_LSU_H
#define NLSR_LSU_H

#include "ns3/header.h"

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

// ========== Class LsuNameList ============

class LsuContent : public Header, public SimpleRefCount<LsuContent> {

public:
  struct NeighborTuple
  {
    std::string routerName;
    uint16_t metric;

    NeighborTuple ()
    {}

    NeighborTuple (std::string n, uint16_t m)
    : routerName (n), metric (m)
    {}
  };

  struct PrefixTuple
  {
    std::string prefixName;
    uint16_t metric;

    PrefixTuple ()
    {}

    PrefixTuple (std::string n, uint16_t m)
    : prefixName (n), metric (m)
    {}
  };

  LsuContent ();
  virtual ~LsuContent ();  

  static TypeId
  GetTypeId (void);

  virtual TypeId
  GetInstanceTypeId (void) const;
  
  void
  Print (std::ostream &os) const;
  
  uint32_t
  GetSerializedSize (void) const;
  
  void
  Serialize (Buffer::Iterator start) const;
  
  uint32_t
  Deserialize (Buffer::Iterator start);

  uint32_t
  GetLifetime () const; 
  
  void
  SetLifetime (uint32_t lifetime); 

  const std::vector<NeighborTuple> &
  GetAdjacency () const;
  
  void
  AddAdjacency (const std::string &routerName, uint16_t metric);

  const std::vector<PrefixTuple> &
  GetReachability () const;
  
  void
  AddReachability (const std::string &prefixName, uint16_t metric);

private:
  uint16_t
  GetAdjacencySize (void) const;

  uint16_t
  GetReachabilitySize (void) const;

private:
  uint32_t m_lifetime; // count-down timer for soft-state protocol 
  std::vector<NeighborTuple> m_adjacency;
  std::vector<PrefixTuple> m_reachability;

}; // class LsuContent

// ========== Class LsuNameList ============
  
class LsuNameList : public Header, public SimpleRefCount<LsuNameList> {

public:

  LsuNameList ();
  LsuNameList (const std::vector<std::string> & nameList);
  virtual ~LsuNameList ();  

  static TypeId
  GetTypeId (void);
  
  virtual TypeId
  GetInstanceTypeId (void) const;
  
  void
  Print (std::ostream &os) const;
  
  uint32_t
  GetSerializedSize (void) const;
  
  void
  Serialize (Buffer::Iterator start) const;
  
  uint32_t
  Deserialize (Buffer::Iterator start);

  const std::vector<std::string> &
  GetNameList () const;

  std::vector<std::string> &
  Get ();
  
  void
  AddName (const std::string & name);

private:
  std::vector<std::string> m_nameList;

}; // class LsuNameList

// ========== Class HelloData ============
  
class HelloData : public Header, public SimpleRefCount<HelloData> {

public:

  HelloData ();
  virtual ~HelloData ();  

  static TypeId
  GetTypeId (void);
  
  virtual TypeId
  GetInstanceTypeId (void) const;
  
  void
  Print (std::ostream &os) const;
  
  uint32_t
  GetSerializedSize (void) const;
  
  void
  Serialize (Buffer::Iterator start) const;
  
  uint32_t
  Deserialize (Buffer::Iterator start);
  
  uint16_t
  GetNeighborListSize (void) const;

  const std::string &
  GetRouterName () const;
  
  void
  SetRouterName (const std::string & routerName);

  const std::vector<std::string> &
  GetNeighborList () const;
  
  void
  AddNeighborList (const std::string & lsuName);

  uint32_t
  GetDeadTime () const;
  
  void
  SetDeadTime (const uint32_t & deadTime);

  uint8_t
  GetVersion () const;
  
  void
  SetVersion (const uint8_t & version);

private:
  std::string m_routerName;
  std::vector<std::string> m_neighborList;
  uint32_t m_deadTime;
  uint8_t m_version;

}; // class LsuNameList

} // namespace nlsr
} // namespace ns3

#endif /* NLSR_LSU_H */
