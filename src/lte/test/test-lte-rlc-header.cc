/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2011, 2012, 2013 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
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
 * Author: Lluis Parcerisa <lparcerisa@cttc.cat> (TestUtils from test-asn1-encoding.cc)
 *         Nicola Baldo <nbaldo@cttc.es> (actual test)
 */

#include "ns3/log.h"
#include "ns3/test.h"
#include "ns3/ptr.h"
#include "ns3/packet.h"

#include "ns3/lte-rlc-am-header.h"

#include <list>
#include <bitset>


NS_LOG_COMPONENT_DEFINE ("TestLteRlcHeader");

namespace ns3 {

class TestUtils
{
public:
  // Function to convert packet contents in hex format
  static std::string sprintPacketContentsHex (Ptr<Packet> pkt)
  {
    uint32_t psize = pkt->GetSize ();
    uint8_t buffer[psize];
    char sbuffer[psize * 3];
    pkt->CopyData (buffer, psize);
    for (uint32_t i = 0; i < psize; i++)
      {
        sprintf (&sbuffer[i * 3],"%02x ",buffer[i]);
      }
    return std::string (sbuffer);
  }

  // Function to convert packet contents in binary format
  static std::string sprintPacketContentsBin (Ptr<Packet> pkt)
  {
    uint32_t psize = pkt->GetSize ();
    uint8_t buffer[psize];
    std::ostringstream oss (std::ostringstream::out);
    pkt->CopyData (buffer, psize);
    for (uint32_t i = 0; i < psize; i++)
      {
        oss << (std::bitset<8> (buffer[i]));
      }
    return std::string (oss.str () + "\n");
  }

  // Function to log packet contents
  static void LogPacketContents (Ptr<Packet> pkt)
  {
    NS_LOG_DEBUG ("---- SERIALIZED PACKET CONTENTS (HEX): -------");
    NS_LOG_DEBUG ("Hex: " << TestUtils::sprintPacketContentsHex (pkt));
    NS_LOG_DEBUG ("Bin: " << TestUtils::sprintPacketContentsBin (pkt));
  }

  template <class T>
  static void LogPacketInfo (T source,std::string s)
  {
    NS_LOG_DEBUG ("--------- " << s.data () << " INFO: -------");
    std::ostringstream oss (std::ostringstream::out);
    source.Print (oss);
    NS_LOG_DEBUG (oss.str ());
  }
};


class RlcAmStatusPduTestCase : public TestCase
{
public:
  RlcAmStatusPduTestCase (SequenceNumber10 ackSn, 
			  std::list<SequenceNumber10> nackSnList,
			  std::string hex);

protected:  
  virtual void DoRun (void);
  
  SequenceNumber10 m_ackSn;  
  std::list<SequenceNumber10> m_nackSnList;
  std::string m_hex;
  
};


RlcAmStatusPduTestCase::RlcAmStatusPduTestCase (SequenceNumber10 ackSn, 
						std::list<SequenceNumber10> nackSnList ,
						std::string hex)
  : TestCase (hex), 
    m_ackSn (ackSn),
    m_nackSnList (nackSnList),
    m_hex (hex)
{
  NS_LOG_FUNCTION (this << hex);
}

void

RlcAmStatusPduTestCase::DoRun ()
{
  NS_LOG_FUNCTION (this);
  
  Ptr<Packet> p = Create<Packet> ();
  LteRlcAmHeader h;
  h.SetAckSn (m_ackSn);
  for (std::list<SequenceNumber10>::iterator it = m_nackSnList.begin ();
       it != m_nackSnList.end ();
       ++it)
    {
      h.PushNack (it->GetValue ());
    }
  p->AddHeader (h);  

  TestUtils::LogPacketContents (p);
  std::string hex = TestUtils::sprintPacketContentsHex (p);
  NS_TEST_ASSERT_MSG_EQ (hex, m_hex, "serialized packet content differs from test vector");
  
  LteRlcAmHeader h2;
  p->RemoveHeader (h2);
  SequenceNumber10 ackSn = h2.GetAckSn ();
  NS_TEST_ASSERT_MSG_EQ (ackSn, m_ackSn, "deserialized ACK SN differs from test vector");
  
  for (std::list<SequenceNumber10>::iterator it = m_nackSnList.begin ();
       it != m_nackSnList.end ();
       ++it)
    {
      int nackSn = h2.PopNack ();
      NS_TEST_ASSERT_MSG_GT (nackSn, -1, "not enough elements in deserialized NACK list");
      NS_TEST_ASSERT_MSG_EQ (nackSn, it->GetValue (), "deserialized NACK SN  differs from test vector");
    }
  int retVal = h2.PopNack ();
  NS_TEST_ASSERT_MSG_LT (retVal, 0, "too many elements in deserialized NACK list");
}


class LteRlcHeaderTestSuite : public TestSuite
{
public:
  LteRlcHeaderTestSuite ();
} staticLteRlcHeaderTestSuiteInstance ;

LteRlcHeaderTestSuite::LteRlcHeaderTestSuite ()
  : TestSuite ("lte-rlc-header", UNIT)
{
  NS_LOG_FUNCTION (this);

  {
    SequenceNumber10 ackSn (8);
    std::list<SequenceNumber10> nackSnList;
    std::string hex ("0020");
    AddTestCase (new RlcAmStatusPduTestCase (ackSn, nackSnList, hex), TestCase::QUICK);
  }

}


} // namespace ns3