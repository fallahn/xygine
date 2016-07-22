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

#ifndef XY_TILELAYER_HPP_
#define XY_TILELAYER_HPP_

#include <xygine/tilemap/Layer.hpp>

namespace xy
{
    namespace tmx
    {
        /*!
        \brief A layer made up from a series of tile sets
        */
        class XY_EXPORT_API TileLayer final : public Layer
        {
        public:
            TileLayer() {};
            ~TileLayer() = default;

            Type getType() const override { return Layer::Type::Tile; }
            void parse(const pugi::xml_node&) override {}
        };
    }
}

#endif //XY_TILE_LAYER_HPP_