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

#ifndef NLSR_SYNC_H
#define NLSR_SYNC_H

#include "ns3/header.h"
#include "ns3/ndn-data.h"

namespace ns3 {
namespace nlsr {

/// ========== Class NlsrSync ============

static const uint64_t INITIAL_DIGEST = 7036231242510567892; //  ns3::Hash64 ("YUZHANG")
typedef std::vector<std::string> NameList;
typedef std::map<std::string, uint64_t> IdSeqMap;

struct LogTuple
{
  uint64_t digest;
  std::string newName;
  std::string oldName;

  LogTuple (uint64_t d, std::string n, std::string o)
  : digest (d), newName (n), oldName (o)
  {}

  LogTuple ()
  {}
};

typedef std::list<LogTuple> DigestLog;

class NlsrSync {

public:

  NlsrSync ();

  virtual ~NlsrSync ();

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
  IsCurrentDigest (uint64_t digest) const;

  bool
  IsDigestInLog (uint64_t digest) const;

  bool
  GetUpdateSinceThen (uint64_t digest, NameList & nameList) const;

  bool
  GetUpdateByThen (uint64_t digest, NameList & nameList) const;

  void
  GetAllName (NameList & nameList) const;

  bool
  Update (const std::string & newName, std::string & oldName);

private:
  void
  AddToLog (uint64_t digest, const std::string & newName, const std::string & oldName);

  uint64_t
  IncrementalHash (const std::string & newName, const std::string & oldName) const;

private:
  DigestLog m_digestLog;
  IdSeqMap m_idSeqMap;
}; // Class NlsrSync


} // namespace nlsr
} // namespace ns3

#endif /* NLSR_SYNC_H */
