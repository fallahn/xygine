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

#ifndef XYT_MATERIAL_EDITOR_STATE_HPP_
#define XYT_MATERIAL_EDITOR_STATE_HPP_

#include <StateIds.hpp>

#include <xygine/State.hpp>
#include <xygine/Resource.hpp>
#include <xygine/ShaderResource.hpp>
#include <xygine/mesh/MaterialResource.hpp>
#include <xygine/mesh/MeshRenderer.hpp>
#include <xygine/Scene.hpp>

#include <SFML/Graphics/Texture.hpp>

#include <array>

namespace xy
{
    class MessageBus;
}
class MaterialEditorState final : public xy::State
{
public:
    MaterialEditorState(xy::StateStack&, Context);
    ~MaterialEditorState();

    bool update(float) override;
    void draw() override;
    bool handleEvent(const sf::Event&) override;
    void handleMessage(const xy::Message&) override;
    xy::StateID stateID() const override
    {
        return States::ID::MaterialEditor;
    }
private:
    xy::MessageBus& m_messageBus;
    xy::TextureResource m_textureResource;
    xy::ShaderResource m_shaderResource;
    xy::MaterialResource m_materialResource;
    xy::Scene m_scene;
    xy::MeshRenderer m_meshRenderer;

    struct Material
    {
        enum
        {
            Coloured,
            Vertex,
            Textured
        }shaderType = Coloured;
        std::array<sf::Uint8, 4> colour = {{255, 255, 255, 255}};
        std::array<std::string, 3> textures;
        bool castShadows = false;
        std::string name= "Untitled";
    };
    std::vector<Material> m_materials;
    Material* m_materialData;

    std::array<sf::Texture, 3> m_textures;
    void clearTexture(std::size_t, const sf::Color&);

    void loadMaterials();
    void buildMenu();

    void loadModel(const std::string& path);
};
#endif //XYT_MATERIAL_EDITOR_STATE_HPP_