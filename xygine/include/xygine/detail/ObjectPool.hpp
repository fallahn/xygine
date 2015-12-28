/*********************************************************************
Matt Marchant 2014 - 2015
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

//creates a fixed size pool of memory from which objects can be allocated

#ifndef XY_OBJECT_POOL_HPP_
#define XY_OBJECT_POOL_HPP_

#include <xygine/Assert.hpp>

#include <vector>
#include <memory>
#include <functional>

namespace xy
{
    namespace Detail
    {
        template <class T>
        class ObjectPool final
        {
        private:
            const std::size_t MAX_BUFFER_SIZE = 2048u;
        public:
            using Ptr = std::unique_ptr<T, std::function<void(T*)>>;
            explicit ObjectPool(std::size_t size)
                : m_poolBuffer  (size * sizeof(T)),
                m_slots         (size),
                m_firstFreeIndex(0)
            {
                XY_ASSERT(size < MAX_BUFFER_SIZE, "max pool size is 2048 objects");

                auto stride = sizeof(T);
                for (auto i = 0u; i < size; ++i)
                {
                    m_slots[i] = std::make_pair(reinterpret_cast<T*>(m_poolBuffer.data() + (i * stride)), true);
                }
            }
            ~ObjectPool() = default;
            explicit ObjectPool(const ObjectPool&) = delete;
            ObjectPool& operator = (const ObjectPool&) = delete;

            //WARNING this returns nullptr if no space is available
            //validity of returned objects should be checked.
            template <typename... Args>
            Ptr get(Args&&... args)
            {
                if (m_firstFreeIndex > -1)
                {
                    Ptr object(new (m_slots[m_firstFreeIndex].first)T(std::forward<Args>(args)...), [this](T* ptr) {freeSlot(ptr); });

                    m_slots[m_firstFreeIndex].second = false;
                    while (m_slots[++m_firstFreeIndex].second == false)
                    {
                        if (m_firstFreeIndex == m_slots.size())
                        {
                            m_firstFreeIndex = -1;
                            break;
                        }
                    }
                    return std::move(object);
                }
                return nullptr;
            }

        private:
            //custom deleter
            void freeSlot(T* ptr)
            {
                //return the ptr to the pool by marking
                //it as an available slot
                for (auto i = 0u; i < m_slots.size(); ++i)
                {
                    if (m_slots[i].first == ptr)
                    {
                        m_slots[i].second = true;

                        //update the first free index
                        if (static_cast<int>(i) < m_firstFreeIndex)
                        {
                            m_firstFreeIndex = i;
                        }

                        break;
                    }
                }
            }

            std::vector<char> m_poolBuffer;
            std::vector<std::pair<T*, bool>> m_slots;
            int m_firstFreeIndex;
        };
    }
}


#endif //XY_OBJECT_POOL_HPP_