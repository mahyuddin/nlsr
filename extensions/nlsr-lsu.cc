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

// ========== Class LsuContent ============

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
  uint32_t size = 0;
  uint16_t smallSize = 0;
  size = sizeof (size) + 
         sizeof (m_lifetime) +
         sizeof (smallSize) + LsuContent::GetAdjacencySize () +
         sizeof (smallSize) + LsuContent::GetReachabilitySize(); 

  NS_LOG_DEBUG ("GetSerializedSize LsuContent: " << size); 

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
  i.WriteHtonU32 (GetSerializedSize ());

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
  uint32_t size = start.ReadNtohU32 ();

  NS_LOG_DEBUG ("Deserialize LsuContent 1:" << size); 

  size -= sizeof (size);

  return Deserialize (start, size) + sizeof (size);
}


uint32_t
LsuContent::Deserialize (Buffer::Iterator start, uint32_t messageSize)
{
  NS_LOG_DEBUG ("Deserialize LsuContent 2: " << messageSize); 

  Buffer::Iterator i = start;
  uint16_t leftSize = messageSize;

  NS_ASSERT (leftSize >= sizeof (m_lifetime));
  m_lifetime = i.ReadNtohU32 ();
  leftSize -= sizeof (m_lifetime);

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

uint32_t
LsuContent::GetLifetime () const
{
  return m_lifetime;
} 

void
LsuContent::SetLifetime (uint32_t lifetime)
{
  m_lifetime = lifetime;
}

const std::vector<LsuContent::NeighborTuple> &
LsuContent::GetAdjacency () const
{
  return m_adjacency;
}
 
void
LsuContent::AddAdjacency (const std::string &routerName, uint16_t metric)
{
  LsuContent::NeighborTuple neighborTuple;
  neighborTuple.routerName = routerName;
  neighborTuple.metric = metric;
  m_adjacency.push_back(neighborTuple);
}

const std::vector<LsuContent::PrefixTuple> & 
LsuContent::GetReachability () const
{
  return m_reachability;
}
 
void
LsuContent::AddReachability (const std::string &prefixName, uint16_t metric)
{
  LsuContent::PrefixTuple prefixTuple;
  prefixTuple.prefixName = prefixName;
  prefixTuple.metric = metric;
  m_reachability.push_back(prefixTuple);
}

// ========== Class LsuNameList ============

NS_OBJECT_ENSURE_REGISTERED (LsuNameList);

LsuNameList::LsuNameList ()
{
}

LsuNameList::~LsuNameList ()
{
}

TypeId
LsuNameList::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::nlsr::LsuNameList")
    .SetParent<Header> ()
    .AddConstructor<LsuNameList> ()
  ;
  return tid;
}

TypeId
LsuNameList::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

uint32_t
LsuNameList::GetSerializedSize (void) const
{
  uint32_t size = 0;
  uint16_t smallSize = 0;

  size += sizeof (size);

  for ( std::vector<std::string>::const_iterator i = m_nameList.begin ();
        i != m_nameList.end ();
        i++ ) {
    size += sizeof (smallSize) + i->size (); 
  }
  NS_LOG_DEBUG ("GetSerializedSize LsuNameList: " << size); 
  return size;
}

void
LsuNameList::Print (std::ostream &os) const
{
  for ( std::vector<std::string>::const_iterator i = m_nameList.begin ();
        i != m_nameList.end ();
        i++ ) {
    os << "LsuName:  " << i->c_str() << "  Length:  " << i->size () << std::endl;
  }
}

void
LsuNameList::Serialize (Buffer::Iterator start) const
{
  Buffer::Iterator i = start;
  i.WriteHtonU32 (GetSerializedSize ());

  for ( std::vector<std::string>::const_iterator name = m_nameList.begin ();
        name != m_nameList.end();
        name++ ) {
    i.WriteHtonU16 (name->size ());
    i.Write ((const uint8_t *) name->c_str (), name->size ());
  }
}

uint32_t
LsuNameList::Deserialize (Buffer::Iterator start)
{

  uint32_t size = start.ReadNtohU32 ();

  NS_LOG_DEBUG ("Deserialize LsuNameList 1:" << size); 

  size -= sizeof (size);

  return Deserialize (start, size) + sizeof (size);
}


uint32_t
LsuNameList::Deserialize (Buffer::Iterator start, uint32_t messageSize)
{
  NS_LOG_DEBUG ("Deserialize LsuNameList 2: " << messageSize); 

  uint32_t leftSize = messageSize;
  Buffer::Iterator i = start;

  while (leftSize > 0) {

    std::string name;
    uint16_t stringSize = 0;

    NS_ASSERT (leftSize >= sizeof (stringSize));
    stringSize = i.ReadNtohU16(); 
    leftSize -= sizeof (stringSize);
     
    NS_LOG_DEBUG ("leftSize: " << leftSize << "  stringSize: " << stringSize);

    NS_ASSERT (leftSize >= stringSize);
    leftSize -= stringSize;
    name.clear ();
    for (; stringSize > 0; stringSize--) {
      name.push_back (i.ReadU8 ()); 
    }

    m_nameList.push_back (name);
  }
 
  NS_ASSERT (leftSize == 0);

  return messageSize;
}

const std::vector<std::string> &
LsuNameList::GetNameList () const
{
  return m_nameList;
}

void
LsuNameList::AddNameList (const std::string &lsuName)
{
  m_nameList.push_back(lsuName);
}
 
} // namespace nlsr
} // namespace ns3

