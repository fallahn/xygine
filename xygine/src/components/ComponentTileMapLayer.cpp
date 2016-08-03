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

#include <xygine/components/TileMapLayer.hpp>
#include <xygine/Entity.hpp>
#include <xygine/Scene.hpp>
#include <xygine/Reports.hpp>
#include <xygine/Resource.hpp>

#include <xygine/detail/GLCheck.hpp>
#include <xygine/detail/GLExtensions.hpp>

#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

#include <limits>

using namespace xy;

namespace
{
    void setTextureProperties(sf::Texture* texture, const std::vector<std::uint16_t>& data, sf::Uint32 x, sf::Uint32 y)
    {
        glCheck(glBindTexture(GL_TEXTURE_2D, texture->getNativeHandle()));
        glCheck(glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16UI, texture->getSize().x, texture->getSize().y, 0, GL_RG_INTEGER, GL_UNSIGNED_SHORT, 0));
        glCheck(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, x, y, GL_RG_INTEGER, GL_UNSIGNED_SHORT, (void*)data.data()));
        glCheck(glBindTexture(GL_TEXTURE_2D, 0));
    }

    const std::string vert =
        "#version 120\n"

        "varying vec2 v_texCoord;\n"
        "varying vec4 v_colour;\n"

        "void main()\n"
        "{\n"
        "    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;"
        "    v_texCoord = (gl_TextureMatrix[0] * gl_MultiTexCoord0).xy;"
        /*"    gl_FrontColor = gl_Color;"*/
        "    v_colour = gl_Color;\n"
        "}";

    const std::string frag =
        "#version 130\n"

        "in vec2 v_texCoord;\n"
        "in vec4 v_colour;\n"

        "uniform usampler2D u_lookup;\n"
        "uniform sampler2D u_tileMap;\n"

        "uniform vec2 u_tileSize;\n"
        "uniform vec2 u_tilesetCount;\n"

        "out vec4 colour;\n"

        "void main()\n"
        "{\n"
        "    uvec2 values = texture(u_lookup, v_texCoord / u_tileSize).rg;\n"
        "    if(values.r > 0u)\n"
        "    {\n"
        "        //colour = vec4(vec3(1.0), 0.33333);\n"
        "        //vec2 size = textureSize(u_tileMap, 0);\n"
        "        float index = float(values.r) - 1.0;\n"
        "        vec2 position = floor(vec2(mod(index, u_tilesetCount.x), index / u_tilesetCount.x)) / u_tilesetCount;\n"
        "        //vec2 offset = mod(u_tileSize, (v_texCoord / u_tileSize));\n"
        "        //vec2 tilePosition = (size / u_tilesetCount) * position;\n"
        "        colour = texture(u_tileMap, position);\n"
        "    }\n"
        "    else\n"
        "    {\n"
        "         colour = vec4(0.0);"
        "    }\n"
        "    colour *= v_colour;\n"
        "}";

    const std::string frag2 =
        "#version 120\n"
        "void main(){gl_FragColor = vec4(gl_TexCoord[0].y, 0.0, 0.0, 1.0);}";
}

TileMapLayer::TileMapLayer(MessageBus& mb, const tmx::Map::Key&, sf::Uint32 chunkResolution)
    : Component(mb, this),
    m_chunkResolution(chunkResolution),
    m_opacity(1.f)
{
    m_testShader.loadFromMemory(vert, frag);
    //m_testShader.loadFromMemory(frag2, sf::Shader::Fragment);
}

//public
void TileMapLayer::entityUpdate(Entity& entity, float)
{
    auto scene = entity.getScene();
    XY_ASSERT(scene, "why you been updated without scene??");

    auto view = scene->getVisibleArea();

    //build list of chunks to render
    m_drawList.clear();
    for (const auto& chunk : m_chunks)
    {
        if (chunk.bounds.intersects(view))
        {
            m_drawList.push_back(&chunk);
        }
    }
}

void TileMapLayer::setTileData(const tmx::TileLayer* layer, const std::vector<tmx::Tileset>& tileSets, const tmx::Map& map, TextureResource& tr)
{
    XY_ASSERT(layer, "not a valid layer");
    
    const auto& tileIDs = layer->getTiles();
    std::uint32_t maxID = std::numeric_limits<std::uint32_t>::max();
    std::vector<const tmx::Tileset*> usedTileSets;

    //discover which tilesets are used in the layer
    for (auto i = tileSets.rbegin(); i != tileSets.rend(); ++i)
    {
        for (const auto& tile : tileIDs)
        {
            if (tile.ID >= i->getFirstGID() && tile.ID < maxID)
            {
                usedTileSets.push_back(&(*i));
                break;
            }
        }
        maxID = i->getFirstGID();
    }

    //divide layer into chunks
    const auto bounds = map.getBounds();
    sf::Uint32 chunkX = static_cast<sf::Uint32>(std::ceil(bounds.width / m_chunkResolution));
    sf::Uint32 chunkY = static_cast<sf::Uint32>(std::ceil(bounds.height / m_chunkResolution));
    float floatRes = static_cast<float>(m_chunkResolution);

    sf::Uint32 tileXCount = m_chunkResolution / map.getTileSize().x;
    sf::Uint32 tileYCount = m_chunkResolution / map.getTileSize().y;
    sf::Uint32 rowCount = static_cast<sf::Uint32>(bounds.width / map.getTileSize().x);

    for (auto y = 0u; y < chunkY; ++y)
    {
        for (auto x = 0u; x < chunkX; ++x)
        {
            //add a new chunk
            m_chunks.emplace_back();
            auto& chunk = m_chunks.back();
            chunk.bounds = { x * floatRes, y * floatRes, floatRes, floatRes };
            chunk.bounds.left += layer->getOffset().x;
            chunk.bounds.top += layer->getOffset().y;
            chunk.vertices = //TODO colour this on layer?
            {
                sf::Vertex(sf::Vector2f(chunk.bounds.left, chunk.bounds.top), sf::Vector2f()),
                sf::Vertex(sf::Vector2f(chunk.bounds.left + floatRes, chunk.bounds.top), sf::Vector2f(floatRes, 0.f)),
                sf::Vertex(sf::Vector2f(chunk.bounds.left + floatRes, chunk.bounds.top + floatRes), sf::Vector2f(floatRes, floatRes)),
                sf::Vertex(sf::Vector2f(chunk.bounds.left, chunk.bounds.top + floatRes), sf::Vector2f(0.f, floatRes))
            };
            
            //check each used tileset, and if it's used by this chunk create a lookup texture
            for (const auto ts : usedTileSets)
            {
                std::vector<std::uint16_t> pixelData;
                bool tsUsed = false;

                auto chunkSize = rowCount * tileYCount;
                auto chunkStart = (y * (tileYCount * rowCount)) + (x * tileXCount); //find starting tile of this chunk
                
                for (auto rowPos = chunkStart; rowPos < chunkStart + chunkSize; rowPos += rowCount)
                {
                    auto colStart = rowPos;
                    for (auto colPos = colStart; colPos < colStart + tileXCount; ++colPos)
                    {
                        if (colPos < tileIDs.size() && tileIDs[colPos].ID >= ts->getFirstGID() 
                            && tileIDs[colPos].ID < (ts->getFirstGID() + ts->getTileCount()))
                        {
                            //ID belongs to this tile set
                            tsUsed = true;
                            pixelData.push_back(static_cast<std::uint16_t>((tileIDs[colPos].ID - ts->getFirstGID()) + 1)); //red channel - making sure to index relative to the tileset
                            pixelData.push_back(static_cast<std::uint16_t>(tileIDs[colPos].flipFlags)); //green channel
                        }
                        else
                        {
                            //pad with empty space else array will be incorrectly aligned
                            pixelData.push_back(0);
                            pixelData.push_back(0);
                        }
                    }
                }

                //if this tileset is used create the texture data for it
                if (tsUsed)
                {
                    chunk.tileData.emplace_back();
                    auto& tileData = chunk.tileData.back();
                    tileData.lookupTexture = std::make_unique<sf::Texture>();
                    tileData.lookupTexture->create(m_chunkResolution, m_chunkResolution);
                    setTextureProperties(tileData.lookupTexture.get(), pixelData, tileXCount, tileYCount);

                    //load / assign tileset texture
                    //TODO shader will need to know tileset data such as margin
                    //and offset. Could use a struct uniform ideally
                    tr.setFallbackColour(sf::Color::Cyan);
                    tileData.tileTexture = &tr.get(ts->getImagePath());
                    tileData.tileCount.x = ts->getTileCount() % ts->getTileSize().x;
                    tileData.tileCount.y = ts->getTileCount() / ts->getTileSize().y;
                }
            }

            //if this chunk ended up empty, remove it (seems wasteful to process in the first place, but ultimately improves drawing)
            if (chunk.tileData.empty()) m_chunks.pop_back();
        }
    }

    m_opacity = layer->getOpacity();
    m_globalBounds = map.getBounds();

    m_testShader.setUniform("u_tileSize", sf::Glsl::Vec2(map.getTileSize()));
}

//private
void TileMapLayer::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{
    //set states shader to tilemap shader
    states.shader = &m_testShader;
    //TODO set shader uniform for layer opacity
    
    //draw each texture in current list
    for (const auto chunk : m_drawList)
    {
        rt.draw(*chunk, states);
    }
}

void TileMapLayer::Chunk::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{
    for (const auto& td : tileData)
    {
        //set shader texture uniforms
        sf::Shader* shader = const_cast<sf::Shader*>(states.shader);
        shader->setUniform("u_lookup", *td.lookupTexture);
        shader->setUniform("u_tileMap", *td.tileTexture);
        shader->setUniform("u_tilesetCount", sf::Glsl::Vec2(td.tileCount));
        states.texture = td.lookupTexture.get();
        rt.draw(vertices.data(), vertices.size(), sf::Quads, states);
    }
}