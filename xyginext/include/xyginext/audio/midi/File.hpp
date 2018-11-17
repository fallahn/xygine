#pragma once

#include "xyginext/Config.hpp"
#include "Track.hpp"

#include <SFML/System/Time.hpp>

#include <fstream>

namespace xy
{
    namespace Midi
    {
        /*!
        \brief Loads a Midi file to be played via xy::Midi::Player.
        */
        class XY_EXPORT_API File final : private std::vector<Track>
        {
        public:
            /*!
            \brief Default constructor.
            */
            File();

            /*
            \brief Constructs a File objects from the midi file
            at the given path, if it is valid.
            \param path Relative path to a midi file on disk
            */
            explicit File(const std::string& path);

            /*
            \brief Attempts to close any previously loaded midi
            files and load the midi file from the given path.
            \param path Relative path to a midi file on disk
            \returns true on success else false on failure
            */
            bool loadFromFile(const std::string& path);

            /*!
            \brief Attempts to load a midi file from the given
            data in memory
            \param data A vector of bytes containing the midi file data
            \returns true on success else returns false
            */
            bool loadFromMemory(const std::vector<std::uint8_t>& data);

            /*!
            \brief Returns the number of tracks in the loaded midi file
            and 0 if no file is loaded
            */
            std::size_t getTrackCount() const;

            /*!
            \brief Returns the number of ticks per quarter note
            */
            std::uint16_t getTimeDivision() const;

            /*!
            \brief Sets the number of ticks per quarter note
            */
            void setTimeDivision(std::uint16_t);

            /*!
            \brief Returns the duration of the loaded midi file.
            If no file is loaded the time will be zero.
            */
            sf::Time getDuration() const;

        private:

            std::uint16_t m_timeDivision;

            bool readHeader(std::fstream&);
            void readTrack(std::fstream&);
            void readEvent(std::fstream&, Event&, bool&, std::uint8_t&);
            void readUnknown(std::fstream&);
        };
    }
}