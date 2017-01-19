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

#ifndef XY_CALLBACK_PROVIDER_HPP_
#define XY_CALLBACK_PROVIDER_HPP_

#include <xygine/components/Component.hpp>

#include <functional>
#include <vector>

namespace xy
{
    /*!
    \brief Utility component for rapid prototyping.
    Sometimes when testing ideas creating a specialised component is
    a lot of extra work involving boilerplate code which may not always
    be necessary. The callback provider component offers an interface
    to implement component functions as a set of one or more callbacks
    using std::function objects. While this is useful to quickly test
    ideas inlined in code it is not as optimal for perfomance as a 
    specialised component class, so it is not recommended to use this
    class for release mode, rather as a utility during development.
    */
    class CallbackProvider final : public xy::Component
    {
    public:
        explicit CallbackProvider(xy::MessageBus& mb) : Component(mb, this) {}
        ~CallbackProvider() = default;

        xy::Component::Type type() const override { return xy::Component::Type::Script; }
        void entityUpdate(Entity&e, float dt) override { for (auto& cb : m_updateCallbacks) cb(e, dt); }
        void onStart(Entity& e) override { for (auto& cb : m_onStartCallbacks) cb(e); }
        void onDelayedStart(Entity& e) override { for (auto& cb : m_delayedStartCallbacks) cb(e); }
        void onParentDestroyed(Entity& e) { for (auto& cb : m_parentDestroyedCallbacks) cb(e); }
        void destroy() override { for (auto& cb : m_destroyCallbacks) cb(); }

        /*!
        \brief Adds a callback to be executed every time the component is updated
        */
        void addUpdateCallback(const std::function<void(Entity&, float)>& cb) { m_updateCallbacks.push_back(cb); }
        /*!
        \brief Adds a callback to be executed when the component's onStart event is fired
        */
        void addOnStartCallback(const std::function<void(Entity&)>& cb) { m_onStartCallbacks.push_back(cb); }
        /*!
        \brief Adds a callback to be executed when the delayedStart event is fired
        */
        void addDelayedStartCallback(const std::function<void(Entity&)>& cb) { m_delayedStartCallbacks.push_back(cb); }
        /*!
        \brief Adds a callback to be executed when the component's parent entity is destroyed
        */
        void addParentDestroyedCallback(const std::function<void(Entity&)>& cb) { m_parentDestroyedCallbacks.push_back(cb); }
        /*!
        \brief Adds a callback to be executed when this component is destroyed
        */
        void addDestroyedCallback(const std::function<void()>& cb) { m_destroyCallbacks.push_back(cb); }

    private:

        std::vector<std::function<void(Entity&, float)>> m_updateCallbacks;
        std::vector<std::function<void(Entity&)>> m_onStartCallbacks;
        std::vector<std::function<void(Entity&)>> m_delayedStartCallbacks;
        std::vector<std::function<void(Entity&)>> m_parentDestroyedCallbacks;
        std::vector<std::function<void()>> m_destroyCallbacks;
    };
}

#endif //XY_CALLBACK_PROVIDER_HPP_