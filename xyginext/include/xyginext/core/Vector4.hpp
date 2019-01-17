/*********************************************************************
(c) Matt Marchant 2017 - 2019
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

#pragma once

namespace xy
{
    template <typename T>
    /*!
    \brief 4 component vector class.
    Useful when storing floating point colour values, particularly with ConfigFile objects
    \see ConfigFile
    */
    class Vector4 final
    {
    public:

        /*!
        \brief Default constructor
        */
        Vector4();

        /*!
        \brief Construct the vector from its coordinates
        */
        Vector4(T X, T Y, T Z, T W);

        /*!
        \brief Construct the vector from another type of vector
        */
        template <typename U>
        explicit Vector4(const Vector4<U>& vector);

        T x; //!< X coordinate of the vector
        T y; //!< Y coordinate of the vector
        T z; //!< Z coordinate of the vector
        T w; //!< W coordinate of the vector
    };

    /*!
    \brief Overload of unary operator -
    */
    template <typename T>
    Vector4<T> operator -(const Vector4<T>& left);

    /*!
    \brief Overload of binary operator +=
    */
    template <typename T>
    Vector4<T>& operator +=(Vector4<T>& left, const Vector4<T>& right);

    /*!
    \brief Overload of binary operator -=
    */
    template <typename T>
    Vector4<T>& operator -=(Vector4<T>& left, const Vector4<T>& right);

    /*!
    \brief Overload of binary operator +
    */
    template <typename T>
    Vector4<T> operator +(const Vector4<T>& left, const Vector4<T>& right);

    /*!
    \brief Overload of binary operator -
    */
    template <typename T>
    Vector4<T> operator -(const Vector4<T>& left, const Vector4<T>& right);

    /*!
    \brief Overload of binary operator *
    */
    template <typename T>
    Vector4<T> operator *(const Vector4<T>& left, T right);

    /*!
    \brief Overload of binary operator *
    */
    template <typename T>
    Vector4<T> operator *(T left, const Vector4<T>& right);

    /*!
    \brief Overload of binary operator *=
    */
    template <typename T>
    Vector4<T>& operator *=(Vector4<T>& left, T right);

    /*!
    \brief Overload of binary operator /
    */
    template <typename T>
    Vector4<T> operator /(const Vector4<T>& left, T right);

    /*!
    \brief Overload of binary operator /=
    */
    template <typename T>
    Vector4<T>& operator /=(Vector4<T>& left, T right);

    /*!
    \brief Overload of binary operator ==
    */
    template <typename T>
    bool operator ==(const Vector4<T>& left, const Vector4<T>& right);

    /*!
    \brief Overload of binary operator !=
    */
    template <typename T>
    bool operator !=(const Vector4<T>& left, const Vector4<T>& right);

#include "Vector4.inl"

    using Vector4i =  Vector4<int>;
    using  Vector4f = Vector4<float>;
}
