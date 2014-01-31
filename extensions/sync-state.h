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

// sync-state.h

#ifndef _SYNC_STATE_H
#define _SYNC_STATE_H

#include "ns3/header.h"
#include "ns3/ndn-data.h"

namespace ns3 {
namespace ndn {

/// ========== Class NlsrSync ============

static const uint64_t INITIAL_DIGEST = 7036231242510567892; //  ns3::Hash64 ("YUZHANG")

struct LogTuple
{
  uint64_t digest;
  std::string newName;
  std::string oldName;
  uint32_t counter;

  LogTuple (uint64_t d, std::string n, std::string o)
  : digest (d), newName (n), oldName (o), counter (0)
  {}

  LogTuple ()
  : counter (0)
  {}
};

typedef std::vector<std::string> NameList;
typedef std::map<std::string, uint64_t> IdSeqMap;
typedef std::list<LogTuple> DigestLog;

class SyncState {

public:

  SyncState ();

  virtual ~SyncState ();

  static TypeId
  GetTypeId (void);

  virtual TypeId
  GetInstanceTypeId (void) const;

  static std::string &
  IdSeqToName (const std::string id, uint64_t seq, std::string & name);

  static void
  NameToIdSeq (const std::string name, std::string & id, uint64_t & seq);

  uint64_t
  GetCurrentDigest () const;

  bool
  IsDigestInLog (uint64_t digest) const;

  uint64_t
  GetSyncDigest () const;

  bool
  IncreaseCounter (uint64_t digest);

  bool
  GetUpdateInbetween (uint64_t oldDigest, uint64_t newDigest, NameList & nameList) const;

  bool
  Update (const std::string & newName, std::string & oldName);

private:

  bool
  IsCurrentDigest (uint64_t digest) const;

  void
  AddToLog (uint64_t digest, const std::string & newName, const std::string & oldName);

  uint64_t
  IncrementalHash (const std::string & newName, const std::string & oldName) const;

  DigestLog::const_iterator
  FindDigestInLog (uint64_t digest) const;

  DigestLog::iterator
  FindDigestInLog (uint64_t digest);

  void
  GetAllName (NameList & nameList) const;

  bool
  GetUpdateSinceThen (uint64_t digest, NameList & nameList) const;

  bool
  GetUpdateByThen (uint64_t digest, NameList & nameList) const;

private:
  DigestLog m_digestLog;
  IdSeqMap m_idSeqMap;
}; // Class SyncState


} // namespace ndn
} // namespace ns3

#endif /* _SYNC_STATE_H */
