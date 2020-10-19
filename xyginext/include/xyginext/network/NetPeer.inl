#pragma once

template <>
inline std::string NetPeer::getAddress<_ENetPeer>() const
{
    IMPL_WARN
    return Detail::getEnetPeerAddress(m_peer);
}

template <>
inline std::uint16_t NetPeer::getPort<_ENetPeer>() const
{
    IMPL_WARN
    return Detail::getEnetPeerPort(m_peer);
}

template <>
inline std::uint64_t NetPeer::getID<_ENetPeer>() const
{
    IMPL_WARN
    return Detail::getEnetPeerID(m_peer);
}

template <>
inline std::uint32_t NetPeer::getRoundTripTime<_ENetPeer>()const
{
    IMPL_WARN
    return Detail::getEnetRoundTrip(m_peer);
}

template <>
inline NetPeer::State NetPeer::getState<_ENetPeer>() const
{
    IMPL_WARN
    return Detail::getEnetPeerState(m_peer);
}

template <>
inline void NetPeer::setPeer(_ENetPeer* peer)
{
    IMPL_WARN
    m_peer = peer;
}
