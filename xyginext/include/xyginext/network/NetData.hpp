/*********************************************************************
(c) Matt Marchant 2017 - 2020
http://trederia.blogspot.com

xygineXT - Zlib license.

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

#pragma once

#include "xyginext/Config.hpp"
#include "xyginext/core/Assert.hpp"
#include <SFML/Config.hpp>

#include <cstring>
#include <array>

//if implementing a custom network api
//defining this will warn if you're using any of
//the default template specialisations instead
//of your own
#ifdef NET_IMPL_WARN
#define IMPL_WARN xy::Logger::Log("Called default network implementation here!", xy::Logger::Type::Warning)
#else
#define IMPL_WARN
#endif //NET_IMPL_WARN

struct _ENetPacket;
struct _ENetPeer;

namespace xy
{
    /*!
    \brief A peer represents a single, multichannel connection between
    a client and a host.
    */
    struct XY_EXPORT_API NetPeer final
    {
        template<typename T = _ENetPeer>
        std::string getAddress() const; //! <String containing the IPv4 address
        template<typename T = _ENetPeer>
        sf::Uint16 getPort() const; //! <Port number
        template<typename T = _ENetPeer>
        sf::Uint64 getID() const; //! <Unique ID
        template<typename T = _ENetPeer>
        sf::Uint32 getRoundTripTime() const; //! <Mean round trip time in milliseconds of a reliable packet

        enum class State
        {
            Disconnected,
            Connecting,
            AcknowledingConnect,
            PendingConnect,
            Succeeded,
            Connected,
            DisconnectLater,
            Disconnecting,
            AcknowledingDisconnect,
            Zombie
        };
        template<typename T = _ENetPeer>
        State getState() const; //! <Current state of the peer

        bool operator == (const NetPeer& other) const
        {
            return other.m_peer == this->m_peer;
        }

        bool operator != (const NetPeer& other) const
        {
            return !(other == *this);
        }

        operator bool() const { return m_peer != nullptr; }

        //specialise this for custom implementations which need
        //to set the peer pointer.
        template <typename T = _ENetPeer>
        void setPeer(T* peer);

        const void* getPeer() const { return m_peer; }

        void reset() { m_peer = nullptr; }

    private:
        void* m_peer = nullptr;

        //friend class EnetHostImpl; //and this one
    };

    namespace Detail
    {
        //used for kludgery - don't call directly!
        XY_EXPORT_API std::string getEnetPeerAddress(void*);
        XY_EXPORT_API sf::Uint16 getEnetPeerPort(void*);
        XY_EXPORT_API sf::Uint32 getEnetPeerID(void*);
        XY_EXPORT_API sf::Uint32 getEnetRoundTrip(void*);
        XY_EXPORT_API NetPeer::State getEnetPeerState(void*);
    }

    /*!
    \brief Network event.
    These are used to poll NetHost and NetClient objects
    for network activity
    */
    struct XY_EXPORT_API NetEvent final
    {
        sf::Uint8 channel = 0; //! <channel event was received on
        enum
        {
            None,
            ClientConnect,
            ClientDisconnect,
            PacketReceived
        }type = None;

        /*!
        \brief Event packet.
        Contains packet data recieved by PacketRecieved event.
        Not valid for other event types.
        */
        struct XY_EXPORT_API Packet final
        {
            Packet();
            ~Packet() = default;

            Packet(const Packet&) = delete;
            Packet(Packet&&) = delete;
            Packet& operator = (const Packet&) = delete;
            Packet& operator = (Packet&&) = delete;

            /*!
            \brief The unique ID this packet was tegged with when sent
            */
            std::uint8_t getID() const;

            /*!
            \brief Used to retreive the data as a specific type.
            Trying to read data as an incorrect type will lead to
            undefined behaviour.
            */
            template <typename T>
            T as() const;

            /*!
            \brief Returns a pointer to the raw packet data
            */
            const void* getData() const;
            /*!
            \brief Returns the size of the data, in bytes
            */
            std::size_t getSize() const;

            /*!
            \brief Used by active implementation to set packet data.
            DO NOT USE DIRECTLY.
            */
            void setPacketData(const std::uint8_t*, std::size_t);
        private:

            std::uint8_t m_id;
            std::vector<std::uint8_t> m_data;
            std::size_t m_size;

        }packet;

        /*!
        \brief Contains the peer from which this event was transmitted
        */
        NetPeer peer;
    };

#include "NetData.inl"
#include "NetPeer.inl"
}
