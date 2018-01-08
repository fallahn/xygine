#ifndef XY_NET_PEER_INL_
#define XY_NET_PEER_INL_

template <>
std::string NetPeer::getAddress<_ENetPeer>() const
{

}

template <>
sf::Uint16 NetPeer::getPort<_ENetPeer>() const
{

}

template <>
sf::Uint32 NetPeer::getID<_ENetPeer>() const
{

}

template <>
sf::Uint32 NetPeer::getRoundTripTime<_ENetPeer>()const
{

}

template <>
NetPeer::State NetPeer::getState<_ENetPeer>() const
{

}

#endif //XY_NET_PEER_INL_
