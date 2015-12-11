project(xygine_example)
cmake_minimum_required(VERSION 2.8)

if(NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build (Debug or Release)" FORCE)
endif()

SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")
SET(xygine_example_STATIC_SFML FALSE CACHE BOOL "Choose whether SFML is linked statically or not.")
SET(xygine_example_STATIC_RUNTIME FALSE CACHE BOOL "Use statically linked standard/runtime libraries? This option must match the one used for SFML.")

#use if you prefer clang, comment out if using g++ (4.9+)
#SET(CMAKE_C_COMPILER    "/usr/bin/clang")
#SET(CMAKE_CXX_COMPILER  "/usr/bin/clang++")
#SET(CMAKE_AR            "/usr/bin/llvm-ar")
#SET(CMAKE_LINKER        "/usr/bin/llvm-ld")
#SET(CMAKE_NM            "/usr/bin/llvm-nm")
#SET(CMAKE_OBJDUMP       "/usr/bin/llvm-objdump")
#SET(CMAKE_RANLIB        "/usr/bin/llvm-ranlib")

if(CMAKE_COMPILER_IS_GNUCXX)
  if(xygine_example_STATIC_RUNTIME)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++1y -static")
  else()
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++1y")
  endif()
endif()

if(xygine_example_STATIC_SFML)
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

#optional package manager on arch (PKGBUILD script currently missing)
#include (FindPkgConfig)
#PKG_SEARCH_MODULE(SFML 2 REQUIRED system window graphics audio)

include_directories(
  ${SFML_INCLUDE_DIR}
  ${CMAKE_SOURCE_DIR}/include
  ${CMAKE_SOURCE_DIR}/../xygine/include)

if(X11_FOUND)
  include_directories(${X11_INCLUDE_DIRS})
endif()


SET(xygine_DIR ${CMAKE_SOURCE_DIR}/../xygine/src)
include(${xygine_DIR}/CMakeLists.txt)

SET(EXAMPLE_SRC
  src/Game.cpp
  src/main.cpp
  src/MenuMainState.cpp
  src/OptionMenuState.cpp
  src/ParticleDemoState.cpp
  src/PhysicsDemoState.cpp)

if(WIN32)
  add_executable(xygine_example WIN32 ${EXAMPLE_SRC} ${xygine_SRC})
else()
  add_executable(xygine_example ${EXAMPLE_SRC} ${xygine_SRC})
endif()

target_link_libraries(xygine_example
  ${SFML_LIBRARIES}
  ${SFML_DEPENDENCIES})

if(UNIX)
  target_link_libraries(xygine_example
    ${X11_LIBRARIES})
endif()

#install executable
install(TARGETS xygine_example
  RUNTIME DESTINATION .)

#install game data
install(DIRECTORY assets
  DESTINATION .)

#install the OpenAL DLL
if(WIN32)
  install(FILES ${SFML_INCLUDE_DIR}/../bin/openal32.dll
    DESTINATION .)
endif()
