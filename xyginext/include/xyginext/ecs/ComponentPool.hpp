/*********************************************************************
(c) Matt Marchant 2017
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

#ifndef XY_POOL_HPP_
#define XY_POOL_HPP_

#include <xyginext/core/Assert.hpp>

#include <vector>

namespace xy
{
	namespace Detail
	{
		class Pool
		{
		public:
			virtual ~Pool() = default;
			virtual void clear() = 0;
		};

		/*!
		\brief memory pooling for components - TODO map component to ID
		*/
		template <class T>
		class ComponentPool final : public Pool
		{
		public:
			explicit ComponentPool(std::size_t size = 100){	resize(size); }

			bool empty() const { return m_pool.empty(); }
			std::size_t size() const { return m_pool.size(); }
			void resize(std::size_t size) { m_pool.resize(size); }
			void clear() { m_pool.clear(); }
			void add(T c) { m_pool.push_back(c); }

            T& at(std::size_t idx) { return m_pool[idx]; }
            const T& at(std::size_t idx) const { return m_pool[idx]; }

			T& operator [] (std::size_t index) { return m_pool[index]; }
			const T& operator [] (std::size_t index) const { return m_pool[index]; }

		private:
			std::vector<T> m_pool;
		};
	}
}

#endif //XY_POOL_HPP_