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

// nlsr-state.h

#ifndef NLSR_STATE_H
#define NLSR_STATE_H

#include "nlsr-lsu.h"
#include "ns3/header.h"
#include "ns3/ndn-data.h"

namespace ns3 {
namespace nlsr {

/// ========== Class LocalProtocolState ============

class LocalProtocolState {

  struct LogTuple
  {
    uint64_t digest;
    std::string lsuName;

    LogTuple (uint64_t d, std::string n)
    : digest (d), lsuName (n)
    {}

    LogTuple ()
    {}
  };

  struct LsuTuple
  {
    uint32_t sequenceNumber;
    Ptr<const LsuContent> lsuContent;
  };

  struct AdjacencyTuple
  {
    std::string router1;
    std::string router2;
    uint16_t metric;
    uint16_t count;
  };

  struct ReachabilityTuple
  {
    std::string router;
    std::string prefix;
    uint16_t metric;
    uint16_t count;
  };

public:

  LocalProtocolState ();

  virtual ~LocalProtocolState ();

  static TypeId
  GetTypeId (void);

  virtual TypeId
  GetInstanceTypeId (void) const;

  uint64_t
  GetCurrentDigest () const;

  void
  AddToLog (uint64_t digest, const std::string & lsuName);

  bool
  IsDigestInLog (uint64_t digest) const;

  bool
  GetUpdateSinceThen (uint64_t digest, Ptr<LsuNameList> lsuNameList) const;

  bool
  ReplaceInLsdb (const std::string & lsuName, 
                 uint32_t newNumber, Ptr<const LsuContent> newContent,
                 uint32_t & oldNumber, Ptr<const LsuContent> oldContent);

  void
  GetAllLsuName (Ptr<LsuNameList> lsuNameList) const;

private:
  std::list<LogTuple> m_digestLog;
  std::map<std::string, LsuTuple> m_lsdb;
  std::vector<AdjacencyTuple> m_adjacency;
  std::vector<ReachabilityTuple> m_reachability;
}; // Class LocalProtocolState


} // namespace nlsr
} // namespace ns3

#endif /* NLSR_STATE_H */
