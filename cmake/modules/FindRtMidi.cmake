#  This will define
#  RTMIDI_FOUND - System has RtMidi
#  RTMIDI_INCLUDE_DIRS - The RtMidi include directories
#  RTMIDI_LIBRARIES - The libraries needed to use RtMidi
#  RTMIDI_DEFINITIONS - Compiler switches required for using RtMidi
#  RTMIDI_VERSION_STRING - The version of RtMidin

find_package(PkgConfig QUIET)
pkg_check_modules(PC_RTMIDI QUIET rtmidi)
set(RTMIDI_DEFINITIONS ${PC_RTMIDI_CFLAGS_OTHER})

find_path(
    RTMIDI_INCLUDE_DIR RtMidi.h
    HINTS ${PC_RTMIDI_INCLUDEDIR} ${PC_RTMIDI_INCLUDE_DIRS}
    PATH_SUFFIXES rtmidi)

find_library(
    RTMIDI_LIBRARY NAMES rtmidi
    HINTS ${PC_RTMIDI_LIBDIR} ${PC_RTMIDI_LIBRARY_DIRS})

# Get version from pkg-config if possible, else scrape it from the header
if(PC_RTMIDI_VERSION)
    set(RTMIDI_VERSION_STRING ${PC_RTMIDI_VERSION})
elseif(RTMIDI_INCLUDE_DIR AND EXISTS "${RTMIDI_INCLUDE_DIR}/RtMidi.h")
    file(STRINGS "${RTMIDI_INCLUDE_DIR}/RtMidi.h" RTMIDI_VERSION_LINE
         REGEX "^#define RTMIDI_VERSION \".*\".*$")
    string(REGEX REPLACE "^#define RTMIDI_VERSION \"(.*)\".*$" "\\1" RTMIDI_VERSION_STRING
         ${RTMIDI_VERSION_LINE})
    unset(RTMIDI_VERSION_LINE)
endif()

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(
    RTMIDI
    REQUIRED_VARS RTMIDI_LIBRARY RTMIDI_INCLUDE_DIR
    VERSION_VAR RTMIDI_VERSION_STRING)

mark_as_advanced(RTMIDI_INCLUDE_DIR RTMIDI_LIBRARY)

if(RTMIDI_FOUND)
    set(RTMIDI_LIBRARIES ${RTMIDI_LIBRARY})
    set(RTMIDI_INCLUDE_DIRS ${RTMIDI_INCLUDE_DIR})
endif()