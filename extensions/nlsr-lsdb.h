/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014 Harbin Institute of Technology, China
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

// nlsr-lsdb.h

#ifndef NLSR_LSDB_H
#define NLSR_LSDB_H

#include "nlsr-lsu.h"
#include "ns3/header.h"
#include "ns3/ndn-data.h"

namespace ns3 {
namespace nlsr {

/// ========== Class LinkStateDatabase ============

class LinkStateDatabase {
  // struct LsuTuple
  // {
  //   uint32_t lsuID;
  // };
 
  // struct LsuTuple
  // {
  //   std::string ;
  //   uint32_t sequenceNumber;
  //   LsuContent lsuContent;
  // };

  // struct AdjacencyTuple
  // {
  //   std::string router1;
  //   std::string router2;
  //   uint16_t metric;
  //   uint16_t count;
  // };

  // struct ReachabilityTuple
  // {
  //   std::string prefix;
  //   std::string router;
  //   uint16_t metric;
  //   uint16_t count;
  // };

public:

  LinkStateDatabase ();

  virtual ~LinkStateDatabase ();

  // std::vector<RouterTuple> m_lsdb;
  // std::vector<AdjacencyTuple> m_adjacency;
  // std::vector<ReachabilityTuple> m_reachability;
}; // Class LinkStateDatabase


} // namespace nlsr
} // namespace ns3

#endif /* NLSR_LSDB_H */
