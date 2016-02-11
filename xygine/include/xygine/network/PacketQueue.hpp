/*********************************************************************
Matt Marchant 2014 - 2016
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

#ifndef XY_PACKET_Q_HPP_
#define XY_PACKET_Q_HPP_

//TODO bear in mind that if this ever becomes a bottleneck
//it should be properly implemented as a circular buffer

#include <xygine/Config.hpp>

#include <SFML/Config.hpp>

#include <list>

namespace xy
{
    namespace Network
    {
        using SeqID = sf::Uint16;

        static inline bool moreRecent(SeqID a, SeqID b, SeqID max)
        {
            sf::Uint16 halfMax = max / 2u;
            return
                (
                    ((a > b) && (a - b <= halfMax))
                    ||
                    ((b > a) && (b - a > halfMax))
                    );
        }

        /*!
        \brief Struct containing information about acknowledged packets
        */
        struct XY_EXPORT_API PacketData final
        {
            SeqID sequence = 0u;
            float timeOffset = 0.f;
            sf::Int32 size = 0;
        };

        /*!
        \brief Used internally for packet data management
        */
        class /*XY_EXPORT_API*/ PacketQueue final : public std::list<xy::Network::PacketData>
        {
        public:
            PacketQueue() = default;
            ~PacketQueue() = default;
            PacketQueue(const PacketQueue&) = delete;
            PacketQueue& operator = (const PacketQueue&) = delete;

            bool exists(SeqID);
            void insertSorted(const PacketData&, SeqID);

            //TODO implement these when converting to proper circular buffer
            /*std::list<PacketData>::iterator begin();
            std::list<PacketData>::iterator end();
            void push_front(const PacketData&);
            void push_back(const PacketData&);
            void pop_front();
            PacketData& front();
            PacketData& back();
            void insert(std::size_t, std::list<PacketData>::iterator);
            std::list<PacketData>::iterator erase(std::list<PacketData>::iterator);
            void clear();
            bool empty();*/

        private:

        };
    }
}
#endif //XY_PACKET_Q_HPP_