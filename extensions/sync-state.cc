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

#include "sync-state.h"
#include "ns3/assert.h"
#include "ns3/log.h"

NS_LOG_COMPONENT_DEFINE ("SyncState");

namespace ns3 {
namespace ndn {

// ========== Class SyncState ============

#define MAX_LOG_LENGTH 10000

SyncState::SyncState ()
{ 
}

SyncState::~SyncState ()
{
}

TypeId
SyncState::GetTypeId (void)
{
  static TypeId tid = TypeId ("SyncState");
  return tid;
}

TypeId
SyncState::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

/// ========================= 

std::string &
SyncState::IdSeqToName (const std::string id, uint64_t seq, std::string & name)
{
  Ptr<ndn::Name> s = Create<ndn::Name> (id);

  s->appendNumber (seq);
  name = s->toUri ();

  return name;
}

void
SyncState::NameToIdSeq (const std::string name, std::string & id, uint64_t & seq)
{
  Ptr<ndn::Name> n = Create<ndn::Name> (name);
  id = n->getPrefix (n->size () - 1).toUri ();
  seq = n->get (-1).toNumber ();
}

uint64_t
SyncState::GetCurrentDigest () const
{
  return m_digestLog.empty ()?  INITIAL_DIGEST : m_digestLog.front ().digest;
}

bool
SyncState::IsCurrentDigest (uint64_t digest) const
{
  return (digest == GetCurrentDigest () ? true : false);
}

bool
SyncState::IsDigestInLog (uint64_t digest) const
{
  return (FindDigestInLog (digest) != m_digestLog.end ());
}

uint64_t
SyncState::GetSyncDigest () const
{
 for (DigestLog::const_iterator i = m_digestLog.begin ();
       i != m_digestLog.end ();
       i++)
  {
    if (i->counter > 1) {
      return i->digest;
    }
  }
  return INITIAL_DIGEST; 
}

bool
SyncState::IncreaseCounter (uint64_t digest)
{
  DigestLog::iterator i = FindDigestInLog (digest);
  if (i != m_digestLog.end ())
  {
      i->counter ++;
      return true;
  }
  return false;
}

uint64_t
SyncState::IncrementalHash (const std::string & newName, const std::string & oldName) const
{
  if (oldName == "") {
    return GetCurrentDigest() ^ ns3::Hash64(newName);
  } else {
    return GetCurrentDigest() ^ ns3::Hash64(newName) ^ ns3::Hash64 (oldName);
  }
}

void
SyncState::AddToLog (uint64_t digest, const std::string & newName, const std::string & oldName)
{
  LogTuple logTuple (digest, newName, oldName);
  m_digestLog.push_front (logTuple);
  NS_LOG_DEBUG ("digest: " << digest << "lsuName: " << newName << "oldName: " << oldName);

  if (m_digestLog.size () > MAX_LOG_LENGTH) {
    m_digestLog.pop_back ();
  }
}

void
SyncState::GetAllName (NameList & nameList) const
{
  std::string str;
  for (IdSeqMap::const_iterator i = m_idSeqMap.begin ();
       i != m_idSeqMap.end ();
       i++)
  {
    nameList.push_back (IdSeqToName (i->first, i->second, str));
  }
}

bool
SyncState::GetUpdateByThen (uint64_t digest, NameList & nameList) const
{
  if (digest == GetCurrentDigest ()) {
    GetAllName (nameList);
    return true;
  } 
  if (IsDigestInLog (digest) == false) {
    return false;
  }
  // To-Do: there must be more efficient way
  std::map<std::string, bool > nameMap;  // the 2nd 'bool' is only a placeholder
  for (IdSeqMap::const_iterator i = m_idSeqMap.begin ();
       i != m_idSeqMap.end ();
       i++)
  {
    std::string str;
    nameMap[IdSeqToName (i->first, i->second, str)] = true;
  }

  for (DigestLog::const_iterator i = m_digestLog.begin ();
       i != m_digestLog.end ();
       i++)
  {
    if (digest != i->digest) {
      nameMap.erase (i->newName);
      nameMap[i->oldName] = true;
    } else {
      break;
    }
  }
  for (std::map<std::string, bool >::const_iterator i = nameMap.begin ();
       i != nameMap.end ();
       i++)
  {
    nameList.push_back (i->first);
  }
  return true;
}

bool
SyncState::GetUpdateInbetween (uint64_t oldDigest, uint64_t newDigest, NameList & nameList) const
{
  if (oldDigest == INITIAL_DIGEST) {
    return GetUpdateByThen (newDigest, nameList);
  }

  // To-Do: there must be more efficient way
  std::map<std::string, bool > nameMap;  // the 2nd 'bool' is only a placeholder

  DigestLog::const_iterator i = m_digestLog.begin ();
  for (; i != m_digestLog.end (); i++)
  {
    if (newDigest == i->digest) break;
  }
  DigestLog::const_iterator j = i;
  for (; j != m_digestLog.end (); j++)
  {
    if (oldDigest == j->digest) break;
    nameMap[j->newName] = true;
  }
  if (j == m_digestLog.end ()) {
    return false;
  }

  for (; j != i; j--) {
    nameMap.erase (j->oldName);
  }
  nameMap.erase (j->oldName);
  for (std::map<std::string, bool >::const_iterator i = nameMap.begin ();
       i != nameMap.end ();
       i++)
  {
    nameList.push_back (i->first);
  }
  return true;
}

bool
SyncState::GetUpdateSinceThen (uint64_t digest, NameList & nameList) const
{
  if (digest == INITIAL_DIGEST) {
    GetAllName (nameList);
    return true;
  } 
  return GetUpdateInbetween (digest, GetCurrentDigest (), nameList);
}

bool
SyncState::Update (const std::string & newName, std::string & oldName)
{
  std::string id;
  uint64_t seq;
  NameToIdSeq (newName, id, seq);

  oldName.clear ();

  IdSeqMap::iterator i = m_idSeqMap.find (id);
  if (i != m_idSeqMap.end ()) {
    if (seq > i->second) {
      IdSeqToName (i->first, i->second, oldName);
      i->second = seq;
      NS_LOG_DEBUG ("New name: " << newName << " Old name: " << oldName);
    } else {
      NS_LOG_DEBUG ("Old name: " << newName);
      return false;
    }
  } else {
    m_idSeqMap[id] = seq;
    NS_LOG_DEBUG ("New name: " << newName);
  }
  AddToLog (IncrementalHash (newName, oldName), newName, oldName);
  return true;
}

DigestLog::const_iterator
SyncState::FindDigestInLog (uint64_t digest) const
{
  DigestLog::const_iterator i = m_digestLog.begin ();
  for (;
       i != m_digestLog.end ();
       i++)
  {
    if (digest == i->digest) {
      break;
    }
  }
  return i;
}

DigestLog::iterator
SyncState::FindDigestInLog (uint64_t digest)
{
  DigestLog::iterator i = m_digestLog.begin ();
  for (;
       i != m_digestLog.end ();
       i++)
  {
    if (digest == i->digest) {
      break;
    }
  }
  return i;
}

} // namespace ndn
} // namespace ns3