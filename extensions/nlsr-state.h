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

/// ========== Class NlsrState ============

class NlsrState {

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

  NlsrState ();

  virtual ~NlsrState ();

  static TypeId
  GetTypeId (void);

  virtual TypeId
  GetInstanceTypeId (void) const;

  uint64_t
  GetCurrentDigest () const;

  static std::string &
  LsuIdSeqToName (const std::string lsuId, uint64_t sequenceNumber, std::string & str);

  static std::string &
  LsuNameToIdSeq (const std::string lsuName, std::string & lsuId, uint64_t & sequenceNumber);

  void
  AddToLog (uint64_t digest, const std::string & lsuName);

  bool
  IsCurrentDigest (uint64_t digest) const;

  bool
  IsDigestInLog (uint64_t digest) const;

  uint64_t
  IncrementalHash (const std::string & newName, const std::string & oldName) const;

  bool
  GetUpdateSinceThen (uint64_t digest, Ptr<LsuNameList> lsuNameList) const;

  bool
  IsNewerLsuName (const std::string & lsuName) const;

  void
  GetAllLsuName (Ptr<LsuNameList> lsuNameList) const;

  uint64_t
  IncrementalHash (const std::string & update) const;

  void
  NewerLsuNameFilter (const std::vector<std::string> & inLsuNameList, std::vector<std::string> & outLsuNameList) const;

  bool
  InsertInLsuIdSeqMap (const std::string & lsuName, std::string & oldName);

  bool
  InsertNewLsu (const std::string & lsuName, Ptr<const LsuContent> newContent);

  void
  AddLsuContent (Ptr<const LsuContent> content);

  void
  RemoveLsuContent (Ptr<const LsuContent> content);

  Ptr<const LsuContent>
  GetLsuContent (const std::string & lsuName) const;

  const std::string &
  GetRouterName () const;

  void
  SetRouterName (const std::string & routerName);

private:
  std::list<LogTuple> m_digestLog;
  std::map<std::string, uint64_t> m_lsuIdSeqMap;
  std::map<std::string, Ptr<const LsuContent> > m_lsdb;
  std::vector<AdjacencyTuple> m_adjacency;
  std::vector<ReachabilityTuple> m_reachability;
  std::string m_routerName;
}; // Class NlsrState


} // namespace nlsr
} // namespace ns3

#endif /* NLSR_STATE_H */
