project(xygine)
cmake_minimum_required(VERSION 2.8)

if(NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build (Debug or Release)" FORCE)
endif()

SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")
SET(xygine_STATIC_SFML FALSE CACHE BOOL "Choose whether SFML is linked statically or not.")
SET(xygine_STATIC_RUNTIME FALSE CACHE BOOL "Use statically linked standard/runtime libraries? This option must match the one used for SFML.")

if(CMAKE_COMPILER_IS_GNUCXX)
  if(xygine_STATIC_RUNTIME)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++1y -static")
  else()
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++1y")
  endif()
endif()

SET (CMAKE_CXX_FLAGS_DEBUG "-g -_DEBUG_")
SET (CMAKE_CXX_FLAGS_RELEASE "-O4 -DNDEBUG")

if(xygine_STATIC_SFML)
  SET(SFML_STATIC_LIBRARIES TRUE)
endif()

if(WIN32)
  find_package(SFML 2 REQUIRED graphics window audio system network main)
else()
  find_package(SFML 2 REQUIRED graphics window audio system network)
endif()

if(UNIX)
  find_package(X11 REQUIRED)
endif()

find_package(BOX2D REQUIRED)

include_directories(
  ${SFML_INCLUDE_DIR}
  ${BOX2D_INCLUDE_DIR}
  ${CMAKE_SOURCE_DIR}/include)

if(X11_FOUND)
  include_directories(${X11_INCLUDE_DIRS})
endif()


SET(xygine_DIR "src")
include("src/CMakeLists.txt")

  
if(WIN32)
  add_library(xygine STATIC WIN32 ${xygine_SRC})
else()
  add_library(xygine STATIC ${xygine_SRC})
endif()

target_link_libraries(xygine
  ${SFML_LIBRARIES}
  ${SFML_DEPENDENCIES}
  ${BOX2D_LIBRARIES})

if(UNIX)
  target_link_libraries(xygine
    ${X11_LIBRARIES})
endif()
