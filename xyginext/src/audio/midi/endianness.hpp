#pragma once

#include <fstream>
#include <cstdint>

namespace xy
{
    namespace Detail
    {
        static inline std::uint32_t getVlq(std::fstream& stream)
        {
            std::uint32_t retVal = 0;
            std::uint8_t c = 0;

            do
            {
                stream.read(reinterpret_cast<char*>(&c), 1);
                retVal = (retVal << 7) + (c & 0x7f);
            } while (c & 0x80);

            return retVal;
        }

        static inline bool platformIsLittleEndian()
        {
            //see https://stackoverflow.com/a/4240014/6740859
            const std::uint16_t i = 1;
            const bool ret = *(std::uint8_t*)&i == 1;
            return ret;
        }

        template <typename T>
        T fixup(const T& v)
        {
            T retVal = 0;

            for (auto i = 0; i < sizeof(v); ++i)
            {
                const T size = static_cast<T>(sizeof(v));
                const T iType = static_cast<T>(i);

                T maskA = 0xff << 8 * i;
                T byte = (v & maskA) >> 8 * i;
                T offset = (size - iType - 1) * 8;
                T maskB = 0xff << offset;
                retVal |= ((byte << offset) & maskB);
            }

            return retVal;
        }

        template <typename T>
        T readBigEndian(std::fstream& file)
        {
            T retVal = 0;
            file.read(reinterpret_cast<char*>(&retVal), sizeof(T));
            if (platformIsLittleEndian())
            {
                retVal = fixup(retVal);
            }
            return retVal;
        }
    }
}