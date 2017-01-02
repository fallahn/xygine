/*********************************************************************
Matt Marchant 2014 - 2017
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

#ifndef XY_ACK_SYSTEM_HPP_
#define XY_ACK_SYSTEM_HPP_

#include <xygine/network/PacketQueue.hpp>

#include <SFML/Network/Packet.hpp>

#include <vector>

namespace xy
{
    /*!
    \brief Networking associated classes
    */
    namespace Network
    {
        /*!
        \brief Used to acknowledge the receiving of packets by either end of a connection
        */
        class XY_EXPORT_API AckSystem final
        {
        public:
            /*!
            \brief Packet header containing current ack information
            */
            struct Header final
            {
                SeqID sequence = 0;
                SeqID ack = 0;
                sf::Uint32 ackBits = 0;
            };

            explicit AckSystem(SeqID maxID = 0xFFFF);
            ~AckSystem() = default;
            AckSystem(const AckSystem&) = delete;
            AckSystem& operator = (AckSystem&) = delete;

            /*!
            \brief Resets the ack system and all internal packet queues
            */
            void reset();
            /*!
            \brief Informs the ack system that a packet was sent
            \param sf::Int32 Size of packet sent
            */
            void packetSent(sf::Int32);
            /*!
            \brief Informs the ack system a packet was received
            \param Header Header information of the received packet
            \param sf::Int32 Size of received packet
            */
            void packetReceived(const Header&, sf::Int32);
            /*!
            \brief Updats the internal state of the ack system.
            \param float delta time (in seconds) since last update
            */
            void update(float);
            /*!
            \brief Returns the current local Sequence ID.
            This is the ID used for the newest packet to be sent
            supplied when creating a new header
            */
            SeqID getLocalSequence() const;
            /*!
            \brief Returns the sequence ID from the last received packet.
            Outgoing packets are tagged with this to acknowledge receipt
            of a packet tagged with this ID
            */
            SeqID getRemoteSequence() const;
            /*!
            \brief Get the maximum SequenceID value before the values
            return to zero
            */
            SeqID getMaxSequence() const;
            /*!
            \brief Returns a vector containing the most recently acked packet IDs
            */
            const std::vector<SeqID>& getAcks() const;
            /*!
            \brief Returns the number of packets sent by this system's connection
            */
            sf::Uint32 getSentPacketCount() const;
            /*!
            \brief Returns the number of packets sent by this system's connection
            */
            sf::Uint32 getReceivedPacketCount() const;
            /*!
            \brief Returns the number of packets considered lost by this system's connection.
            By default up to 32 newer packets are received before a packet is considered lost
            */
            sf::Uint32 getLostPacketCount() const;
            /*!
            \brief Returns the total number of packets acknowledged by this system's connection
            */
            sf::Uint32 getAckedPacketCount() const;

            float getSentBandwidth() const;
            float getAckedBandwidth() const;
            /*!
            \brief Returns the round trip time (on average) of this system's connection.
            Also known as ping this returns the value in milliseconds
            */
            float getRoundTripTime() const;
            /*!
            \brief Returns the most current header to tag packets for acknowledgement
            */
            Header createHeader();
        private:

            SeqID m_maxID;
            SeqID m_localSequence;
            SeqID m_remoteSequence;

            sf::Uint32 m_sentPacketCount;
            sf::Uint32 m_receivedPacketCount;
            sf::Uint32 m_lostPacketCount;
            sf::Uint32 m_ackedPacketCount;

            float m_sentBandwidth;
            float m_ackedBandwidth;
            float m_rtt;

            std::vector<SeqID> m_acks;
            PacketQueue m_sentQueue;
            PacketQueue m_pendingAckQueue;
            PacketQueue m_receivedQueue;
            PacketQueue m_ackedQueue;

            void processAck(SeqID, sf::Uint32);
            void advanceQueueTime(float);
            void updateQueues();
            void updateStats();
        };
    }
}
sf::Packet& operator <<(sf::Packet& packet, const xy::Network::AckSystem::Header& header);

sf::Packet& operator >>(sf::Packet& packet, xy::Network::AckSystem::Header& header);

#endif //XY_ACK_SYSTEM_HPP_