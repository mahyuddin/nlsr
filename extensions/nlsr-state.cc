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

// nlsr-lsdb.cc

#include "nlsr-state.h"
#include "ns3/assert.h"
#include "ns3/log.h"

NS_LOG_COMPONENT_DEFINE ("NlsrState");

namespace ns3 {
namespace nlsr {

// ========== Class NlsrState ============

NlsrState::NlsrState ()
{ 
  // std::string name;
  // LsuIdSeqToName ("/router1/lsu1", 1, name);
  // LogTuple logTuple (3428090803022502957, name);
  // m_digestLog.push_front (logTuple);
  // NS_LOG_DEBUG ("Initial NlsrState");
  //InsertNewLsu ("/router1/lsu1/%01", 0);
  //InsertNewLsu ("/router1/lsu2/%02", 0);
}

NlsrState::~NlsrState ()
{
}

TypeId
NlsrState::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::nlsr::NlsrState");
  return tid;
}

TypeId
NlsrState::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

uint64_t
NlsrState::GetCurrentDigest () const
{
  //NS_ASSERT (m_digestLog.front ().digest);
  //NS_LOG_DEBUG ("Digest: " << m_digestLog.front ().digest);
  return m_digestLog.empty ()?  ns3::Hash64 ("/nlsr/resync") : m_digestLog.front ().digest;
}

bool
NlsrState::IsCurrentDigest (uint64_t digest) const
{
  return (digest == GetCurrentDigest () ? true : false);
}

std::string &
NlsrState::LsuIdSeqToName (const std::string lsuId, uint64_t sequenceNumber, std::string & name)
{
  Ptr<ndn::Name> s = Create<ndn::Name> (lsuId);
  s->appendNumber (sequenceNumber);
  name = s->toUri ();
  NS_LOG_DEBUG ("lsuId " << lsuId << " seq " << sequenceNumber << " name " << name);
  return name;
}

std::string &
NlsrState::LsuNameToIdSeq (const std::string lsuName, std::string & lsuId, uint64_t & sequenceNumber)
{
  Ptr<ndn::Name> name = Create<ndn::Name> (lsuName);
  //NS_LOG_DEBUG ("lsuName: " << lsuName);
  lsuId = name->getPrefix (name->size () - 1).toUri ();
  //NS_LOG_DEBUG ("lsuId: " << lsuId);
  sequenceNumber = name->get (-1).toNumber ();
  //NS_LOG_DEBUG ("lsuId " << lsuId << " seq " << sequenceNumber << " name " << lsuName);
  return lsuId;
}

void
NlsrState::AddToLog (uint64_t digest, const std::string & lsuName)
{
  LogTuple logTuple (digest, lsuName);
  m_digestLog.push_front (logTuple);

  if (m_digestLog.size () > 100) {
    m_digestLog.pop_back ();
  }
}

bool
NlsrState::IsDigestInLog (uint64_t digest) const
{
  for (std::list<LogTuple>::const_iterator i = m_digestLog.begin ();
       i != m_digestLog.end ();
       i++)
  {
    if (digest == i->digest) {
      return true;
    }
  }
  return false;
}

bool
NlsrState::GetUpdateSinceThen (uint64_t digest, Ptr<LsuNameList> lsuNameList) const
{
  for (std::list<LogTuple>::const_iterator i = m_digestLog.begin ();
       i != m_digestLog.end ();
       i++)
  {
    if (digest != i->digest) {
      lsuNameList->AddName (i->lsuName);
    } else {
      return true;
    }
  }
  return false;
}

uint64_t
NlsrState::IncrementalHash (const std::string & newName, const std::string & oldName) const
{
  if (oldName == "") {
    return GetCurrentDigest() | ns3::Hash64(newName);
  } else {
    return GetCurrentDigest() | ns3::Hash64(newName) | ns3::Hash64 (oldName);
  }
}

bool
NlsrState::IsNewerLsuName (const std::string & lsuName) const
{
  NS_LOG_DEBUG ("Test string: " << lsuName);
  std::string lsuId;
  uint64_t seq;
  LsuNameToIdSeq (lsuName, lsuId, seq);
  std::map<std::string, uint64_t>::const_iterator i = m_lsuIdSeqMap.find (lsuId);
  if (i != m_lsuIdSeqMap.end ()) {
    if (seq > i->second) {
      return true;
    } else {
      return false;
    }
  } 
  return true;
}

void
NlsrState::NewerLsuNameFilter (const std::vector<std::string> & inLsuNameList, std::vector<std::string> & outLsuNameList) const
{
  for (std::vector<std::string>::const_iterator i = inLsuNameList.begin ();
       i != inLsuNameList.end ();
       i++ )
  {
    if (IsNewerLsuName (*i)) {
      outLsuNameList.push_back (*i);
      NS_LOG_DEBUG ("NewerLsuName: " << *i);
    }
  }
}

bool
NlsrState::InsertInLsuIdSeqMap (const std::string & lsuName, std::string & oldName)
{
  std::string lsuId;
  uint64_t seq;
  LsuNameToIdSeq (lsuName, lsuId, seq);
  std::map<std::string, uint64_t>::iterator i = m_lsuIdSeqMap.find (lsuId);
  if (i != m_lsuIdSeqMap.end ()) {
    if (seq > i->second) {
      i->second = seq;
      LsuIdSeqToName (i->first, i->second, oldName);
      return true;
    } else {
      return false;
    }
  }
  m_lsuIdSeqMap[lsuId] = seq;
  oldName = "";
  return true;
}

void
NlsrState::AddLsuContent (Ptr<const LsuContent> content)
{
  const std::vector<LsuContent::NeighborTuple> & adjacency = content->GetAdjacency ();
  const std::vector<LsuContent::PrefixTuple> & reachability = content->GetReachability ();
}

void
NlsrState::RemoveLsuContent (Ptr<const LsuContent> content)
{
  const std::vector<LsuContent::NeighborTuple> & adjacency = content->GetAdjacency ();
  const std::vector<LsuContent::PrefixTuple> & reachability = content->GetReachability ();
}

bool
NlsrState::InsertNewLsu (const std::string & lsuName, Ptr<const LsuContent> newContent)
{
  std::string oldName = "";
  NS_LOG_DEBUG ("lsuName: " << lsuName);
  if (not InsertInLsuIdSeqMap (lsuName, oldName)) {
    return false;
  }
  AddToLog (IncrementalHash (lsuName, oldName), lsuName);
  m_lsdb[lsuName] = newContent;
  AddLsuContent (newContent);
  if (oldName != "") {
    std::map<std::string, Ptr<const LsuContent> >::iterator i = m_lsdb.find (lsuName);
    NS_ASSERT (i != m_lsdb.end ());
    RemoveLsuContent (i->second);
    m_lsdb.erase (i);
  }
  return true;
}

void
NlsrState::GetAllLsuName (Ptr<LsuNameList> lsuNameList) const
{
  for (std::map<std::string, Ptr<const LsuContent> >::const_iterator i = m_lsdb.begin ();
       i != m_lsdb.end ();
       i++)
  {
    lsuNameList->AddName (i->first);
  }
}

Ptr<const LsuContent>
NlsrState::GetLsuContent (const std::string & lsuName) const
{
  if (IsNewerLsuName (lsuName)) {
    return 0;
  } else {
    return m_lsdb.find (lsuName)->second;
  }
}

const std::string &
NlsrState::GetRouterName () const
{
  return m_routerName;
}

void
NlsrState::SetRouterName (const std::string & routerName)
{
  m_routerName = routerName;
}

} // namespace nlsr
} // namespace ns3