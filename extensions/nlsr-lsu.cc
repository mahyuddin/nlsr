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
  os << "=== LsuContent ===" << std::endl;
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
  Buffer::Iterator i = start;
  i.WriteHtonU32 (GetSerializedSize () - sizeof (uint32_t));

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
  uint32_t messageSize = start.ReadNtohU32 ();

  //NS_LOG_DEBUG ("Deserialize LsuContent:" << messageSize); 

  uint32_t leftSize = messageSize;
  Buffer::Iterator i = start;

  NS_ASSERT (leftSize >= sizeof (m_lifetime));
  m_lifetime = i.ReadNtohU32 ();
  leftSize -= sizeof (m_lifetime);

  uint16_t adjacencySize = i.ReadNtohU16 ();
  //NS_LOG_DEBUG ("adjacencySize: " << adjacencySize << "  leftSize: " << leftSize); 

  leftSize -= sizeof (adjacencySize);
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
  //NS_LOG_DEBUG ("reachabilitySize: " << reachabilitySize << "  leftSize: " << leftSize); 

  leftSize -= sizeof (reachabilitySize);
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

  return messageSize + sizeof (messageSize);
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
  NeighborTuple neighborTuple (routerName, metric);
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
  PrefixTuple prefixTuple (prefixName, metric);
  m_reachability.push_back(prefixTuple);
}

// ========== Class LsuNameList ============

NS_OBJECT_ENSURE_REGISTERED (LsuNameList);

LsuNameList::LsuNameList ()
{
}

LsuNameList::LsuNameList (const std::vector<std::string> & nameList)
{
   std::copy (nameList.begin(), nameList.end (), m_nameList.begin ());
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
  os << "=== LsuNameList ===" << std::endl;
  for ( std::vector<std::string>::const_iterator i = m_nameList.begin ();
        i != m_nameList.end ();
        i++ ) {
    //os << "LsuName:  " << i->c_str() << "  Length:  " << i->size () << std::endl;
    os << "LsuName:  " << i->c_str() << std::endl;
  }
}

void
LsuNameList::Serialize (Buffer::Iterator start) const
{
  Buffer::Iterator i = start;
  i.WriteHtonU32 (GetSerializedSize () - sizeof (uint32_t));

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

  uint32_t messageSize = start.ReadNtohU32 ();

  //NS_LOG_DEBUG ("Deserialize LsuNameList 1:" << messageSize); 

  uint32_t leftSize = messageSize;
  Buffer::Iterator i = start;

  while (leftSize > 0) {

    std::string name;
    uint16_t stringSize = 0;

    NS_ASSERT (leftSize >= sizeof (stringSize));
    stringSize = i.ReadNtohU16(); 
    leftSize -= sizeof (stringSize);
     
    //NS_LOG_DEBUG ("leftSize: " << leftSize << "  stringSize: " << stringSize);

    NS_ASSERT (leftSize >= stringSize);
    leftSize -= stringSize;
    name.clear ();
    for (; stringSize > 0; stringSize--) {
      name.push_back (i.ReadU8 ()); 
    }

    m_nameList.push_back (name);
  }
 
  NS_ASSERT (leftSize == 0);

  return messageSize + sizeof (messageSize);
}

const std::vector<std::string> &
LsuNameList::GetNameList () const
{
  return m_nameList;
}

std::vector<std::string> &
LsuNameList::Get ()
{
  return m_nameList;
}

void
LsuNameList::AddName (const std::string &lsuName)
{
  m_nameList.push_back(lsuName);
}
 
// ========== Class HelloData ============

NS_OBJECT_ENSURE_REGISTERED (HelloData);

HelloData::HelloData ()
{
}

HelloData::~HelloData ()
{
}

TypeId
HelloData::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::nlsr::HelloData")
    .SetParent<Header> ()
    .AddConstructor<HelloData> ()
  ;
  return tid;
}

TypeId
HelloData::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

//  std::string m_routerName;
//  std::vector<std::string> m_neighborList;
//  uint32_t m_deadTime;
//  uint8_t m_version;


uint16_t
HelloData::GetNeighborListSize (void) const
{
  uint16_t size = 0;
  for ( std::vector<std::string>::const_iterator i = m_neighborList.begin ();
        i != m_neighborList.end ();
        i++ ) {
    size += sizeof (uint16_t) + i->size (); 
  }
  return size;
}

uint32_t
HelloData::GetSerializedSize (void) const
{
  uint32_t size = 0;

  size += sizeof (uint32_t);
  size += sizeof (uint16_t) + m_routerName.size ();
  size += sizeof (uint16_t) + GetNeighborListSize ();
  size += sizeof (m_deadTime) + sizeof (m_version);

  NS_LOG_DEBUG ("GetSerializedSize HelloData: " << size); 
  return size;
}

void
HelloData::Print (std::ostream &os) const
{
  os << "=== HelloData ===" << std::endl;
  os << "RouterName:  " << m_routerName << std::endl;

  for ( std::vector<std::string>::const_iterator i = m_neighborList.begin ();
        i != m_neighborList.end ();
        i++ ) {
    os << "NeighborName:  " << i->c_str() << "  Length:  " << i->size () << std::endl;
  }

  os << "DeadTime:  " << m_deadTime<< std::endl;
  os << "Version:  " << (uint16_t) m_version << std::endl;
}

void
HelloData::Serialize (Buffer::Iterator start) const
{
  Buffer::Iterator i = start;
  i.WriteHtonU32 (GetSerializedSize () - sizeof (uint32_t));

  i.WriteHtonU16 (m_routerName.size ());
  i.Write ((const uint8_t *) m_routerName.c_str (), m_routerName.size ());  

  i.WriteHtonU16 (GetNeighborListSize ());
  for ( std::vector<std::string>::const_iterator name = m_neighborList.begin ();
        name != m_neighborList.end();
        name++ ) {
    i.WriteHtonU16 (name->size ());
    i.Write ((const uint8_t *) name->c_str (), name->size ());
  }
  i.WriteHtonU32 (m_deadTime);
  i.WriteU8 (m_version);
}

uint32_t
HelloData::Deserialize (Buffer::Iterator start)
{

  uint32_t messageSize = start.ReadNtohU32 ();

  //NS_LOG_DEBUG ("Deserialize HelloData:" << messageSize); 

  uint32_t leftSize = messageSize;
  Buffer::Iterator i = start;

  uint16_t nameSize = 0;
  nameSize = i.ReadNtohU16 ();
  //NS_LOG_DEBUG ("leftSize: " << leftSize << "  nameSize: " << nameSize);
  leftSize -= (sizeof (uint16_t) + nameSize);

  for (; nameSize > 0; nameSize--) {
      m_routerName.push_back (i.ReadU8 ()); 
  }
   
  uint16_t neighborListSize = i.ReadNtohU16 ();
  leftSize -= ( sizeof (uint16_t) + neighborListSize );

  while (neighborListSize > 0) {

    std::string name;
    uint16_t stringSize = 0;

    NS_ASSERT (neighborListSize >= sizeof (stringSize));
    stringSize = i.ReadNtohU16 (); 
    neighborListSize -= sizeof (stringSize);
    //NS_LOG_DEBUG ("neighborListSize: " << neighborListSize << "  stringSize: " << stringSize);
     
    NS_ASSERT (neighborListSize >= stringSize);
    neighborListSize -= stringSize;
    name.clear ();
    for (; stringSize > 0; stringSize--) {
      name.push_back (i.ReadU8 ()); 
    }

    m_neighborList.push_back (name);
  }
 
  m_deadTime = i.ReadNtohU32 ();
  m_version = i.ReadU8 ();

  leftSize -= sizeof (m_deadTime) + sizeof (m_version);

  NS_ASSERT (leftSize == 0);

  return messageSize + sizeof (messageSize);
}

const std::string &
HelloData::GetRouterName () const
{
  return m_routerName;
}

void
HelloData::SetRouterName (const std::string & routerName)
{
  m_routerName = routerName;
}

const std::vector<std::string> &
HelloData::GetNeighborList () const
{
  return m_neighborList;
}

void
HelloData::AddNeighborList (const std::string & neighborName)
{
  m_neighborList.push_back (neighborName);
}

uint32_t
HelloData::GetDeadTime () const
{
  return m_deadTime;
}

void
HelloData::SetDeadTime (const uint32_t & deadTime)
{
  m_deadTime = deadTime;
}

uint8_t
HelloData::GetVersion () const
{
  return m_version;
}

void
HelloData::SetVersion (const uint8_t & version)
{
  m_version = version;
}

} // namespace nlsr
} // namespace ns3

