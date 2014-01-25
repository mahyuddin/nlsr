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

#include "nlsr-lsu.h"
#include "ns3/assert.h"
#include "ns3/log.h"

NS_LOG_COMPONENT_DEFINE ("NlsrLsu");

namespace ns3 {
namespace nlsr {

NS_OBJECT_ENSURE_REGISTERED (LsuContent);

uint16_t
LsuContent::GetAdjacencySize (void) const
{
  uint16_t size = 0;
  for ( std::vector<nlsr::LsuContent::NeighborTuple>::const_iterator i = m_adjacency.begin ();
        i != m_adjacency.end ();
        i++ ) {
    size += sizeof (i->metric) + sizeof (size) + i->routerName.size(); 
  }
  return size;
}

uint16_t
LsuContent::GetReachabilitySize (void) const
{
  uint16_t size = 0;
  for ( std::vector<nlsr::LsuContent::PrefixTuple>::const_iterator i = m_reachability.begin ();
        i != m_reachability.end ();
        i++ ) {
    size += sizeof (i->metric) + sizeof (size) + i->prefixName.size(); 
  }
  return size;
}


LsuContent::LsuContent ()
{
}

LsuContent::~LsuContent ()
{
}

TypeId
LsuContent::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::nlsr::LsuContent")
    .SetParent<Header> ()
    .AddConstructor<LsuContent> ()
  ;
  return tid;
}

TypeId
LsuContent::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

uint32_t
LsuContent::GetSerializedSize (void) const
{
  NS_LOG_DEBUG ("GetSerializedSize LsuContent"); 

  uint32_t size = 0;
  uint16_t smallSize = 0;
  size += sizeof (m_lifetime) + sizeof (smallSize) + LsuContent::GetAdjacencySize () + sizeof (smallSize) + LsuContent::GetReachabilitySize(); 
  return size;
}

void
LsuContent::Print (std::ostream &os) const
{
  os << "Lifetime:  " << m_lifetime << std::endl;
  os << "AdjacencySize:  " << GetAdjacencySize () << std::endl;
  for ( std::vector<nlsr::LsuContent::NeighborTuple>::const_iterator neighborTuple = m_adjacency.begin ();
        neighborTuple != m_adjacency.end();
        neighborTuple++ ) {
    os << "RouterName:  " << neighborTuple->routerName << "  Length:  " << neighborTuple->routerName.size ()
       << "  Metric:  " << neighborTuple->metric << std::endl;
  }

  os << "ReachabilitySize:  " << GetReachabilitySize () << std::endl;
  for ( std::vector<nlsr::LsuContent::PrefixTuple>::const_iterator prefixTuple = m_reachability.begin ();
        prefixTuple != m_reachability.end();
        prefixTuple++ ) {
    os << "PrefixName:  " << prefixTuple->prefixName << "  Length:  " << prefixTuple->prefixName.size ()
       << "  Metric:  " << prefixTuple->metric << std::endl;
  }
}

void
LsuContent::Serialize (Buffer::Iterator start) const
{
  NS_LOG_DEBUG ("Serialize LsuContent"); 

  Buffer::Iterator i = start;
  i.WriteHtonU32 (m_lifetime);

  i.WriteHtonU16 (GetAdjacencySize ());
  for ( std::vector<nlsr::LsuContent::NeighborTuple>::const_iterator neighborTuple = m_adjacency.begin ();
        neighborTuple != m_adjacency.end();
        neighborTuple++ ) {
    i.WriteHtonU16 (neighborTuple->metric);
    i.WriteHtonU16 (neighborTuple->routerName.size ());
    i.Write ((const uint8_t *) neighborTuple->routerName.c_str (), neighborTuple->routerName.size());
  }

  i.WriteHtonU16 (GetReachabilitySize ());
  for ( std::vector<nlsr::LsuContent::PrefixTuple>::const_iterator prefixTuple = m_reachability.begin ();
        prefixTuple != m_reachability.end();
        prefixTuple++ ) {
    i.WriteHtonU16 (prefixTuple->metric);
    i.WriteHtonU16 (prefixTuple->prefixName.size ());
    i.Write ((const uint8_t *) prefixTuple->prefixName.c_str (), prefixTuple->prefixName.size());
  }
}

uint32_t
LsuContent::Deserialize (Buffer::Iterator start)
{
  NS_LOG_DEBUG ("Deserialize LsuContent 1"); 

  return Deserialize (start, start.GetSize ());
}


uint32_t
LsuContent::Deserialize (Buffer::Iterator start, uint32_t messageSize)
{
  NS_LOG_DEBUG ("Deserialize LsuContent 2"); 

  Buffer::Iterator i = start;

  NS_ASSERT (messageSize >= sizeof (m_lifetime));
  m_lifetime = i.ReadNtohU32 ();
 
  uint16_t leftSize = messageSize - sizeof (m_lifetime);

  NS_ASSERT (leftSize >= sizeof (leftSize));
  uint16_t adjacencySize = i.ReadNtohU16 ();
  leftSize -= sizeof (leftSize);
  NS_ASSERT (leftSize >= adjacencySize);
  leftSize -= adjacencySize;

  while (adjacencySize > 0) {
    NeighborTuple neighborTuple;

    NS_ASSERT (adjacencySize >= sizeof (neighborTuple.metric));
    neighborTuple.metric = i.ReadNtohU16(); 
    adjacencySize -= sizeof (neighborTuple.metric);

    uint16_t stringSize = 0;

    NS_ASSERT (adjacencySize >= sizeof (stringSize));
    stringSize = i.ReadNtohU16(); 
    adjacencySize -= sizeof (stringSize);

    NS_ASSERT (adjacencySize >= stringSize);
    adjacencySize -= stringSize;
    neighborTuple.routerName.clear ();
    for (; stringSize > 0; stringSize--) {
      neighborTuple.routerName.push_back (i.ReadU8 ()); 
    }

    m_adjacency.push_back (neighborTuple);
  }
 
  NS_ASSERT (leftSize >= sizeof (leftSize));
  uint16_t reachabilitySize = i.ReadNtohU16 ();
  leftSize -= sizeof (leftSize);
  NS_ASSERT (leftSize >= reachabilitySize);
  leftSize -= reachabilitySize;

  while (reachabilitySize > 0) {
    PrefixTuple prefixTuple;

    NS_ASSERT (reachabilitySize >= sizeof (prefixTuple.metric));
    prefixTuple.metric = i.ReadNtohU16(); 
    reachabilitySize -= sizeof (prefixTuple.metric);

    uint16_t stringSize = 0;
    NS_ASSERT (reachabilitySize >= sizeof (stringSize));
    stringSize = i.ReadNtohU16(); 
    reachabilitySize -= sizeof (stringSize);

    NS_ASSERT (reachabilitySize >= stringSize);
    reachabilitySize -= stringSize;
    prefixTuple.prefixName.clear ();
    for (; stringSize > 0; stringSize--) {
      prefixTuple.prefixName.push_back (i.ReadU8 ()); 
    }

    m_reachability.push_back (prefixTuple);
  }
 
  NS_ASSERT (leftSize == 0);

  return messageSize;
}

uint32_t LsuContent::GetLifetime () const
{
  return m_lifetime;
} 

void LsuContent::SetLifetime (uint32_t lifetime)
{
  m_lifetime = lifetime;
}

const std::vector<LsuContent::NeighborTuple> & LsuContent::GetAdjacency () const
{
  return m_adjacency;
}
 
void LsuContent::AddAdjacency (const std::string &routerName, uint16_t metric)
{
  LsuContent::NeighborTuple neighborTuple;
  neighborTuple.routerName = routerName;
  neighborTuple.metric = metric;
  m_adjacency.push_back(neighborTuple);
}

const std::vector<LsuContent::PrefixTuple> & LsuContent::GetReachability () const
{
  return m_reachability;
}
 
void LsuContent::AddReachability (const std::string &prefixName, uint16_t metric)
{
  LsuContent::PrefixTuple prefixTuple;
  prefixTuple.prefixName = prefixName;
  prefixTuple.metric = metric;
  m_reachability.push_back(prefixTuple);
}
 
} // namespace nlsr
} // namespace ns3

