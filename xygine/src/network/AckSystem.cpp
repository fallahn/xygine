/*********************************************************************
© Matt Marchant 2014 - 2017
http://trederia.blogspot.com

xygine - Zlib license.

This software is provided 'as-is', without any express or
implied warranty. In no event will the authors be held
liable for any damages arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute
it freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented;
you must not claim that you wrote the original software.
If you use this software in a product, an acknowledgment
in the product documentation would be appreciated but
is not required.

2. Altered source versions must be plainly marked as such,
and must not be misrepresented as being the original software.

3. This notice may not be removed or altered from any
source distribution.
*********************************************************************/

#include <xygine/network/AckSystem.hpp>
#include <xygine/Assert.hpp>


using namespace xy::Network;

namespace
{
    const sf::Uint32 maxAckSize = 33u;
    const float maxRtt = 1.f;

    sf::Int32 bitIndexInSequence(SeqID sequence, SeqID ack, SeqID maxID)
    {
        XY_ASSERT(sequence != ack, "sequence and ack cannot be the same");
        XY_ASSERT(!moreRecent(sequence, ack, maxID), "");
        if (sequence > ack)
        {
            XY_ASSERT(ack < 33, "cannot have more than 32 acks");
            XY_ASSERT(maxID >= sequence, "sequence number too large");
            return ack + (maxID - sequence);
        }
        else
        {
            XY_ASSERT(ack >= 1, "cannot be first ack");
            XY_ASSERT(sequence <= ack - 1, "");
            return ack - 1 - sequence;
        }
    }

    sf::Uint32 genAckBits(SeqID ack, const PacketQueue& receivedQueue, SeqID maxID)
    {
        sf::Uint32 ackBits = 0u;

        for (const auto& pd : receivedQueue)
        {
            if (pd.sequence == ack || moreRecent(pd.sequence, ack, maxID)) break;

            auto bitIdx = bitIndexInSequence(pd.sequence, ack, maxID);
            if (bitIdx < 32) ackBits |= (1 << bitIdx);
        }
        return ackBits;
    }

    void procAck(SeqID ack, sf::Uint32 ackBits, PacketQueue& pendingAcks, PacketQueue& ackedQueue,
        std::vector<SeqID>& acks, sf::Uint32& ackedPackets, float& rtt, SeqID maxID)
    {
        if (pendingAcks.empty()) return;

        auto it = pendingAcks.begin();
        while (it != pendingAcks.end())
        {
            bool acked = false;

            if (it->sequence == ack)
            {
                acked = true;
            }
            else if (!moreRecent(it->sequence, ack, maxID))
            {
                auto bitIdx = bitIndexInSequence(it->sequence, ack, maxID);
                if (bitIdx < 32) acked = (ackBits >> bitIdx) & 1;
            }

            if (acked)
            {
                rtt += (it->timeOffset - rtt) * 0.1f;

                ackedQueue.insertSorted(*it, maxID);
                acks.push_back(it->sequence);
                ackedPackets++;
                it = pendingAcks.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }
}

AckSystem::AckSystem(SeqID maxID)
    : m_maxID               (maxID),
    m_localSequence         (0u),
    m_remoteSequence        (0u),
    m_sentPacketCount       (0u),
    m_receivedPacketCount   (0u),
    m_lostPacketCount       (0u),
    m_ackedPacketCount      (0u),
    m_sentBandwidth         (0.f),
    m_ackedBandwidth        (0.f),
    m_rtt                   (0.f)
{

}

//public
void AckSystem::reset()
{
    m_localSequence = 0u;
    m_remoteSequence = 0u;
    m_sentPacketCount = 0u;
    m_receivedPacketCount = 0u;
    m_lostPacketCount = 0u;
    m_ackedPacketCount = 0u;
    m_sentBandwidth = 0.f;
    m_ackedBandwidth = 0.f;
    m_rtt = 0.f;

    m_sentQueue.clear();
    m_pendingAckQueue.clear();
    m_receivedQueue.clear();
    m_ackedQueue.clear();
}

void AckSystem::packetSent(sf::Int32 size)
{
    XY_ASSERT(!m_sentQueue.exists(m_localSequence), "already acked");
    XY_ASSERT(!m_pendingAckQueue.exists(m_localSequence), "already pending");

    PacketData pd;
    pd.sequence = m_localSequence;
    pd.size = size;
    m_sentQueue.push_back(pd);
    m_pendingAckQueue.push_back(pd);
    m_sentPacketCount++;
    m_localSequence++;

    if (m_localSequence > m_maxID) m_localSequence = 0u;
}

void AckSystem::packetReceived(const Header& header, sf::Int32 size)
{
    m_receivedPacketCount++;
    if (m_receivedQueue.exists(header.sequence)) return;

    PacketData pd;
    pd.sequence = header.sequence;
    pd.size = size;
    m_receivedQueue.push_back(pd);

    if (moreRecent(header.sequence, m_remoteSequence, m_maxID))
    {
        m_remoteSequence = header.sequence;
    }

    processAck(header.ack, header.ackBits);
}

void AckSystem::update(float dt)
{
    m_acks.clear();
    advanceQueueTime(dt);
    updateQueues();
    updateStats();
}

SeqID AckSystem::getLocalSequence() const
{
    return m_localSequence;
}

SeqID AckSystem::getRemoteSequence() const
{
    return m_remoteSequence;
}

SeqID AckSystem::getMaxSequence() const
{
    return m_maxID;
}

const std::vector<SeqID>& AckSystem::getAcks() const
{
    return m_acks;
}

sf::Uint32 AckSystem::getSentPacketCount() const
{
    return m_sentPacketCount;
}

sf::Uint32 AckSystem::getReceivedPacketCount() const
{
    return m_receivedPacketCount;
}

sf::Uint32 AckSystem::getLostPacketCount() const
{
    return m_lostPacketCount;
}

sf::Uint32 AckSystem::getAckedPacketCount() const
{
    return m_ackedPacketCount;
}

float AckSystem::getSentBandwidth() const
{
    return m_sentBandwidth;
}

float AckSystem::getAckedBandwidth() const
{
    return m_ackedBandwidth;
}

float AckSystem::getRoundTripTime() const
{
    return m_rtt;
}

AckSystem::Header AckSystem::createHeader()
{
    Header header;
    header.sequence = m_localSequence;
    header.ack = m_remoteSequence;
    header.ackBits = genAckBits(getRemoteSequence(), m_receivedQueue, m_maxID);
    return header;
}

//private
void AckSystem::processAck(SeqID ack, sf::Uint32 ackBits)
{
    return procAck(ack, ackBits, m_pendingAckQueue, m_ackedQueue, m_acks, m_ackedPacketCount, m_rtt, m_maxID);
}

void AckSystem::advanceQueueTime(float dt)
{
    for (auto& pd : m_sentQueue) pd.timeOffset += dt;
    for (auto& pd : m_receivedQueue) pd.timeOffset += dt;
    for (auto& pd : m_pendingAckQueue) pd.timeOffset += dt;
    for (auto& pd : m_ackedQueue) pd.timeOffset += dt;
}

void AckSystem::updateQueues()
{
    const float epsilon = 0.001f;

    while (!m_sentQueue.empty() && m_sentQueue.front().timeOffset > maxRtt + epsilon)
    {
        m_sentQueue.pop_front();
    }

    if (!m_receivedQueue.empty())
    {
        const auto latestSeq = m_receivedQueue.back().sequence;
        const auto minSeq = (latestSeq > 33) ?
            latestSeq - 34 :
            m_maxID - (34 - latestSeq);

        while (!m_receivedQueue.empty() && !moreRecent(m_receivedQueue.front().sequence, minSeq, m_maxID))
        {
            m_receivedQueue.pop_front();
        }
    }

    while (!m_ackedQueue.empty() && m_ackedQueue.front().timeOffset > maxRtt * 2.f - epsilon)
    {
        m_ackedQueue.pop_front();
    }

    while (!m_pendingAckQueue.empty() && m_pendingAckQueue.front().timeOffset > maxRtt + epsilon)
    {
        m_pendingAckQueue.pop_front();
        m_lostPacketCount++;
    }
}

void AckSystem::updateStats()
{
    sf::Int32 sentBytesPerSec = 0;
    for (const auto& pd : m_sentQueue)
    {
        sentBytesPerSec += pd.size;
    }

    //sf::Int32 ackedPacketsPerSec = 0;
    sf::Int32 ackedBytesPerSec = 0;
    for (const auto& pd : m_ackedQueue)
    {
        if (pd.timeOffset >= maxRtt)
        {
            //ackedPacketsPerSec++;
            ackedBytesPerSec += pd.size;
        }
    }

    m_sentBandwidth = static_cast<float>(sentBytesPerSec) * (8.f / 1000.f);
    m_ackedBandwidth = static_cast<float>(ackedBytesPerSec) * (8.f / 1000.f);
}

//----------packet operator-------//
sf::Packet& operator <<(sf::Packet& packet, const xy::Network::AckSystem::Header& header)
{
    return packet << header.sequence << header.ack << header.ackBits;
}

sf::Packet& operator >>(sf::Packet& packet, xy::Network::AckSystem::Header& header)
{
    return packet >> header.sequence >> header.ack >> header.ackBits;
}