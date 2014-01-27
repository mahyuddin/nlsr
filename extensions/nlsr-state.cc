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

// ========== Class LocalProtocolState ============

LocalProtocolState::LocalProtocolState ()
{  
}

LocalProtocolState::~LocalProtocolState ()
{
}

TypeId
LocalProtocolState::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::nlsr::LocalProtocolState");
  return tid;
}

TypeId
LocalProtocolState::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

uint64_t
LocalProtocolState::GetCurrentDigest () const
{
  NS_ASSERT (m_digestLog.front ().digest);
  return m_digestLog.front ().digest;
}

void
LocalProtocolState::AddToLog (uint64_t digest, const std::string & lsuName)
{
  LogTuple logTuple (digest, lsuName);
  m_digestLog.push_front (logTuple);

  if (m_digestLog.size () > 100) {
    m_digestLog.pop_back ();
  }
}

bool
LocalProtocolState::IsDigestInLog (uint64_t digest) const
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
LocalProtocolState::GetUpdateSinceThen (uint64_t digest, Ptr<LsuNameList> lsuNameList) const
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

bool
LocalProtocolState::ReplaceInLsdb (const std::string & lsuName, 
                                  uint32_t newNumber, Ptr<const LsuContent> newContent,
                                  uint32_t & oldNumber, Ptr<const LsuContent> oldContent)
{
  std::map<std::string, LsuTuple>::iterator i = m_lsdb.find (lsuName);
  if (i != m_lsdb.end ()) {
    if (newNumber > i->second.sequenceNumber) {
      oldNumber = i->second.sequenceNumber;
      i->second.sequenceNumber = oldNumber;
      oldContent = i->second.lsuContent;
      i->second.lsuContent = newContent;
      return true;
    }
  }
  return false;
}

void
LocalProtocolState::GetAllLsuName (Ptr<LsuNameList> lsuNameList) const
{
  for (std::map<std::string, LsuTuple>::const_iterator i = m_lsdb.begin ();
       i != m_lsdb.end ();
       i++)
  {
    Ptr<ndn::Name> s = Create<ndn::Name> (i->first);
    s->appendNumber (i->second.sequenceNumber);
    lsuNameList->AddName (s->toUri ());
  }
}

} // namespace nlsr
} // namespace ns3