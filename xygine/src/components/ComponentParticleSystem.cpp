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

#include <xygine/components/ParticleSystem.hpp>
#include <xygine/util/Random.hpp>
#include <xygine/util/Vector.hpp>
#include <xygine/Entity.hpp>
#include <xygine/MessageBus.hpp>
#include <xygine/Assert.hpp>

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Shader.hpp>
#include <SFML/Graphics/Texture.hpp>

namespace
{

}

using namespace xy;

ParticleSystem::ParticleSystem(MessageBus& mb)
    : Component         (mb, this),
    m_texture           (nullptr),
    m_colour            (sf::Color::White),
    m_followParent      (false),
    m_particleSize      (4.f, 4.f),
    m_particleLifetime  (2.f),
    m_startDelay        (0.f),
    m_randVelocity      (false),
    m_emitRate          (30.f),
    m_randPosition      (false),
    m_started           (false),
    m_accumulator       (0.f),
    m_destroyWhenStopped(false),
    m_needsUpdate       (true),
    m_bounds            (0.f, 0.f, 1.f, 1.f),
    m_duration          (0.f),
    m_releaseCount      (1u),
    m_blendMode         (sf::BlendAdd),
    m_shader            (nullptr)
{

}

//public
Component::Type ParticleSystem::type() const
{
    return Component::Type::Drawable;
}

void ParticleSystem::entityUpdate(Entity& entity, float dt)
{
    update(dt);
    if(!m_followParent)
        m_position = entity.getWorldPosition();

    //if (m_destroyWhenStopped && !active())
    //{
    //    destroy();
    //}
}

void ParticleSystem::onParentDestroyed(Entity& entity)
{   
   
}

sf::FloatRect ParticleSystem::localBounds() const
{
    return m_bounds;
}

sf::FloatRect ParticleSystem::globalBounds() const
{
    return m_bounds;
}

void ParticleSystem::setTexture(const sf::Texture& t)
{
    m_texture = const_cast<sf::Texture*>(&t);
    m_texCoords = sf::Vector2f(m_texture->getSize());
}

void ParticleSystem::setColour(const sf::Color& colour)
{
    m_colour = colour;
}

void ParticleSystem::setBlendMode(sf::BlendMode mode)
{
    m_blendMode = mode;
}

void ParticleSystem::setShader(sf::Shader& shader)
{
    m_shader = &shader;
}

void ParticleSystem::setParticleSize(const sf::Vector2f& size)
{
    m_particleSize = size;
}

void ParticleSystem::setPosition(const sf::Vector2f& position)
{
    m_position = position;
}

void ParticleSystem::move(const sf::Vector2f& amount)
{
    m_position += amount;
}

void ParticleSystem::followParent(bool follow)
{
    m_followParent = follow;
}

void ParticleSystem::setParticleLifetime(float time)
{
    XY_ASSERT(time > 0, "invalid time value");
    m_particleLifetime = time;
}

void ParticleSystem::setInitialVelocity(const sf::Vector2f& vel)
{
    m_initialVelocity = vel;
}

void ParticleSystem::setRandomInitialVelocity(const std::vector<sf::Vector2f>& randVelocities)
{
    //XY_ASSERT(!randVelocities.empty(), "random velocity vector contains no values");
    if (randVelocities.empty()) return;
    m_randVelocities = randVelocities;
    m_randVelocity = true;
}

void ParticleSystem::setEmitRate(float rate)
{
    XY_ASSERT(rate > 0.f, "invalid emit rate");
    m_emitRate = rate;
}

void ParticleSystem::setRandomInitialPosition(const std::vector<sf::Vector2f>& positions)
{
    //XY_ASSERT(!positions.empty(), "position vetor contains no values");
    if (positions.empty()) return;
    m_randPositions = positions;
    m_randPosition = true;
}

void ParticleSystem::addAffector(Affector& a)
{
    m_affectors.push_back(a);
}

void ParticleSystem::start(sf::Uint8 releaseCount, float startDelay, float duration)
{
    XY_ASSERT(releaseCount > 0, "release count not greater than zero");
    //XY_ASSERT(duration >= 0.f, "duration is less than zero");
    m_releaseCount = releaseCount;
    m_duration = duration;
    m_durationClock.restart();
    m_startDelay = startDelay;

    m_started = true;
}

bool ParticleSystem::started() const
{
    return m_started;
}

void ParticleSystem::stop()
{
    m_started = false;
}

bool ParticleSystem::active() const
{
    return (m_particles.size() > 0);
}

void ParticleSystem::update(float dt)
{
    //remove dead particles
    while (!m_particles.empty() && m_particles.front().lifetime <= 0)
    {
        m_particles.pop_front();
    }

    for (auto& p : m_particles)
    {
        p.lifetime -= dt;
        p.move(p.velocity * dt);
        for (auto& a : m_affectors)
        {
            a(p, dt);
        }
    }

    m_needsUpdate = true;

    if (m_started)
    {
        if (m_startDelay > 0)
        {
            m_startDelay -= dt;
            return;
        }

        emit(dt);
        if (m_duration > 0)
        {
            if (m_durationClock.getElapsedTime().asSeconds() > m_duration)
            {
                m_started = false;
            }
        }
    }
}

sf::Uint32 ParticleSystem::getParticleCount() const
{
    return m_particles.size();
}

//private
void ParticleSystem::emit(float dt)
{
    const float interval = 1.f / m_emitRate;

    m_accumulator += dt;
    while (m_accumulator > interval)
    {
        m_accumulator -= interval;
        for (auto i = 0u; i < m_releaseCount; ++i)
        {
            m_randPosition ? 
                addParticle(m_position + m_randPositions[Util::Random::value(0, m_randPositions.size() - 1)]) :
                addParticle(m_position);
        }
    }
}

void ParticleSystem::addParticle(const sf::Vector2f& position)
{
    Particle p;
    p.setPosition(position);
    p.colour = m_colour;
    p.lifetime = m_particleLifetime;
    p.velocity = (m_randVelocity) ? 
        m_randVelocities[Util::Random::value(0, m_randVelocities.size() - 1)] :
        m_initialVelocity;

    m_particles.push_back(p);
}

void ParticleSystem::addVertex(const sf::Vector2f& position, float u, float v, const sf::Color& colour) const
{
    sf::Vertex vert;
    vert.position = position;
    vert.texCoords = { u, v };
    vert.color = colour;

    m_vertices.push_back(vert);

    auto relPos = position - m_position;
    if (relPos.x < m_bounds.left) m_bounds.left = relPos.x;
    if (relPos.y < m_bounds.top) m_bounds.top = relPos.y;
}

void ParticleSystem::updateVertices() const
{
    sf::Vector2f halfSize = m_particleSize / 2.f;

    m_bounds = sf::FloatRect();

    m_vertices.clear();
    for (auto& p : m_particles)
    {
        auto colour = p.colour;

        //make particle fade based on lifetime
        float ratio = p.lifetime / m_particleLifetime;
        colour.a = static_cast<sf::Uint8>(255.f * std::max(ratio, 0.f));

        auto t = p.getTransform();
        addVertex(t.transformPoint(-halfSize.x, -halfSize.y), 0.f, 0.f, colour);
        addVertex(t.transformPoint(halfSize.x, -halfSize.y), m_texCoords.x, 0.f, colour);
        addVertex(t.transformPoint(halfSize), m_texCoords.x, m_texCoords.y, colour);
        addVertex(t.transformPoint(-halfSize.x, halfSize.y), 0.f, m_texCoords.y, colour);
    }

    for (const auto& vert : m_vertices)
    {
        auto width = vert.position.x - (m_bounds.left + m_position.x);
        if (width > m_bounds.width) m_bounds.width = width;

        auto height = vert.position.y - (m_bounds.top + m_position.y);
        if (height > m_bounds.height) m_bounds.height = height;
    }
 
}

void ParticleSystem::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{
    if (m_needsUpdate)
    {
        updateVertices();
        m_needsUpdate = false;
    }

    if (m_shader)
    {
        m_shader->setParameter("u_diffuseMap", sf::Shader::CurrentTexture);
    }

    states.texture = m_texture;
    states.shader = m_shader;
    states.blendMode = m_blendMode;
    if (!m_followParent)states.transform = sf::Transform::Identity;
    rt.draw(m_vertices.data(), m_vertices.size(), sf::PrimitiveType::Quads, states);
}


#include <xygine/util/Json.hpp>
#include <xygine/parsers/picojson.h>
#include <xygine/Resource.hpp>
//-------particle system definition------//
ParticleSystem::Definition::Definition()
{

}

void ParticleSystem::Definition::loadFromFile(const std::string& path, TextureResource& textureResource)
{
    reset();

    //load / parse json file
    std::ifstream file(path);
    if (!file.good() || !Util::File::validLength(file))
    {
        LOG("failed to open " + path + ", or file empty", Logger::Type::Error);
        file.close();
        return;
    }

    std::string jsonString;
    while (!file.eof())
    {
        std::string temp;
        file >> temp;
        jsonString += temp;
    }
    if (jsonString.empty())
    {
        LOG(path + "failed to read, or file empty", Logger::Type::Error);
        file.close();
        return;
    }
    file.close();

    picojson::value pv;
    auto err = picojson::parse(pv, jsonString);
    if (err.empty())
    {
        //int ReleaseCount
        if (pv.get("ReleaseCount").is<double>())
        {
            releaseCount = static_cast<sf::Uint8>(pv.get("ReleaseCount").get<double>());
        }
        //float Delay
        if (pv.get("Delay").is<double>())
        {
            delay = static_cast<float>(pv.get("Delay").get<double>());
        }
        //float Duration
        if (pv.get("Duration").is<double>())
        {
            duration = static_cast<float>(pv.get("Duration").get<double>());
        }
        //string Texture
        if (pv.get("Texture").is<std::string>())
        {
            std::string texturePath = pv.get("Texture").get<std::string>();
            if(!texturePath.empty()) texture = &textureResource.get(texturePath);
        }
        //int Colour
        if (pv.get("Colour").is<double>())
        {
            colour = Util::Json::colourFromInt(static_cast<sf::Int32>(pv.get("Colour").get<double>()));
        }
        //string BlendMode Add, Multiply, Alpha, None
        if (pv.get("BlendMode").is<std::string>())
        {
            std::string mode = pv.get("BlendMode").get<std::string>();
            if (mode == "Add") blendMode = sf::BlendAdd;
            else if (mode == "Multiply") blendMode = sf::BlendMultiply;
            else if (mode == "Alpha") blendMode = sf::BlendAlpha;
            else blendMode = sf::BlendNone;
        }
        //string pair ParticleSize
        if (pv.get("ParticleSize").is<std::string>())
        {
            particleSize = Util::Vector::vec2FromString<float>(pv.get("ParticleSize").get<std::string>());

        }
        //string pair ParticlePosition
        if (pv.get("ParticlePosition").is<std::string>())
        {
            particlePosition = Util::Vector::vec2FromString<float>(pv.get("ParticlePosition").get<std::string>());
        }
        //bool FollowParent
        if (pv.get("FollowParent").is<bool>())
        {
            followParent = pv.get("FollowParent").get<bool>();
        }
        //float Lifetime
        if (pv.get("Lifetime").is<double>())
        {
            lifetime = static_cast<float>(pv.get("Lifetime").get<double>());
        }
        //string pair InitialVelocity
        if (pv.get("InitialVelocity").is<std::string>())
        {
            initialVelocity = Util::Vector::vec2FromString<float>(pv.get("InitialVelocity").get<std::string>());
        }
        //array string pair RandomInitialVelocities
        if (pv.get("RandomInitialVelocities").is<picojson::array>())
        {
            const auto& arr = pv.get("RandomInitialVelocities").get<picojson::array>();
            if (!arr.empty())
            {
                for (const auto& v : arr)
                {
                    if (v.is<std::string>())
                    {
                        randomInitialVelocities.push_back(Util::Vector::vec2FromString<float>(v.get<std::string>()));
                    }
                }
            }
        }
        //float EmitRate
        if (pv.get("EmitRate").is<double>())
        {
            emitRate = static_cast<float>(pv.get("EmitRate").get<double>());
        }
        //array string pair RandomInitialPositions
        if (pv.get("RandomInitialPositions").is<picojson::array>())
        {
            const auto& arr = pv.get("RandomInitialPositions").get<picojson::array>();
            if (!arr.empty())
            {
                for (const auto& v : arr)
                {
                    if (v.is<std::string>())
                    {
                        randomInitialPositions.push_back(Util::Vector::vec2FromString<float>(v.get<std::string>()));
                    }
                }
            }
        }
        //array Affectors
        if (pv.get("Affectors").is<picojson::array>())
        {
            const auto& arr = pv.get("Affectors").get<picojson::array>();
            if (!arr.empty())
            {
                for (const auto& v : arr)
                {
                    //string Type Force, Scale, Colour, Rotation
                    std::string type;
                    if (v.get("Type").is<std::string>()) type = v.get("Type").get<std::string>();
                    
                    if (type == "Force")
                    {
                        if (v.get("Data").is<picojson::array>())
                        {
                            const auto& d = v.get("Data").get<picojson::array>();
                            if (d.size() > 1)
                            {
                                sf::Vector2f force;
                                if (d[0].is<double>()) force.x = static_cast<float>(d[0].get<double>());
                                if (d[1].is<double>()) force.y = static_cast<float>(d[1].get<double>());
                                affectors.emplace_back(ForceAffector(force));
                            }
                        }
                    }
                    else if (type == "Scale")
                    {
                        if (v.get("Data").is<picojson::array>())
                        {
                            const auto& d = v.get("Data").get<picojson::array>();
                            if (d.size() > 1)
                            {
                                sf::Vector2f scale;
                                if (d[0].is<double>()) scale.x = static_cast<float>(d[0].get<double>());
                                if (d[1].is<double>()) scale.y = static_cast<float>(d[1].get<double>());
                                affectors.emplace_back(ScaleAffector(scale));
                            }
                        }
                    }
                    else if (type == "Colour")
                    {
                        if (v.get("Data").is<picojson::array>())
                        {
                            const auto& d = v.get("Data").get<picojson::array>();
                            if (d.size() > 2)
                            {
                                sf::Color start, end;
                                if (d[0].is<double>()) start = Util::Json::colourFromInt(static_cast<sf::Int32>(d[0].get<double>()));
                                if (d[1].is<double>()) end = Util::Json::colourFromInt(static_cast<sf::Int32>(d[1].get<double>()));
                                float cd = 1.f;
                                if (d[2].is<double>()) cd = static_cast<float>(d[2].get<double>());
                                affectors.emplace_back(ColourAffector(start, end, cd));
                            }
                        }
                    }
                    else if (type == "Rotation")
                    {
                        if (v.get("Data").is<picojson::array>())
                        {
                            const auto& d = v.get("Data").get<picojson::array>();
                            if (!d.empty())
                            {
                                float rot = 0.f;
                                if (d[0].is<double>()) rot = static_cast<float>(d[0].get<double>());
                                affectors.emplace_back(RotateAffector(rot));
                            }
                        }
                    }
                }
            }
        }
    }
    else
    {
        Logger::log("Particle Definition: " + err, Logger::Type::Error, Logger::Output::All);
    }
}

ParticleSystem::Ptr ParticleSystem::Definition::createSystem(MessageBus& mb) const
{
    auto ps = Component::create<ParticleSystem>(mb);

    if (texture) ps->setTexture(*texture);
    ps->setColour(colour);
    ps->setBlendMode(blendMode);
    if (shader) ps->setShader(*shader);

    ps->setParticleSize(particleSize);
    ps->setPosition(particlePosition);
    ps->followParent(followParent);

    ps->setParticleLifetime(lifetime);
    ps->setInitialVelocity(initialVelocity);
    ps->setRandomInitialVelocity(randomInitialVelocities);
    ps->setEmitRate(emitRate);
    ps->setRandomInitialPosition(randomInitialPositions);

    for (const auto& a : affectors) ps->addAffector(a);

    return std::move(ps);
}

//private
void ParticleSystem::Definition::reset()
{
    releaseCount = 1u;
    delay = 0.f;
    duration = 1.f;
    texture = nullptr;
    colour = sf::Color::White;
    blendMode = sf::BlendAlpha;
    shader = nullptr;
    particleSize = { 4.f, 4.f };
    particlePosition = {};
    followParent = false;
    lifetime = 2.f;
    initialVelocity = { 1.f, 1.f };
    randomInitialVelocities.clear();
    randomInitialPositions.clear();
    affectors.clear();
}
