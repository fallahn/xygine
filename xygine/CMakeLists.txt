project(xygine)
cmake_minimum_required(VERSION 2.8)

if(NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build (Debug or Release)" FORCE)
endif()

SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")
SET(xygine_STATIC_SFML FALSE CACHE BOOL "Choose whether SFML is linked statically or not.")
SET(xygine_STATIC_RUNTIME FALSE CACHE BOOL "Use statically linked standard/runtime libraries? This option must match the one used for SFML.")

#use if you prefer clang, comment out if using g++ (4.9+)
#SET(CMAKE_C_COMPILER    "/usr/bin/clang")
#SET(CMAKE_CXX_COMPILER  "/usr/bin/clang++")
#SET(CMAKE_AR            "/usr/bin/llvm-ar")
#SET(CMAKE_LINKER        "/usr/bin/llvm-ld")
#SET(CMAKE_NM            "/usr/bin/llvm-nm")
#SET(CMAKE_OBJDUMP       "/usr/bin/llvm-objdump")
#SET(CMAKE_RANLIB        "/usr/bin/llvm-ranlib")

if(CMAKE_COMPILER_IS_GNUCXX)
  if(xygine_STATIC_RUNTIME)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++1y -static")
  else()
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++1y")
  endif()
endif()

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

#optional package manager on arch (PKGBUILD script currently missing)
#include (FindPkgConfig)
#PKG_SEARCH_MODULE(SFML 2 REQUIRED system window graphics audio)

include_directories(
  ${SFML_INCLUDE_DIR}
  ${CMAKE_SOURCE_DIR}/include)

if(X11_FOUND)
  include_directories(${X11_INCLUDE_DIRS})
endif()

set(SOURCE
  src/Affectors.cpp
  src/AnimatedDrawable.cpp
  src/AnimationController.cpp
  src/App.cpp
  src/AudioManager.cpp
  src/CommandQueue.cpp
  src/Component.cpp
  src/Entity.cpp
  src/FileSystem.cpp
  src/FontResource.cpp
  src/MessageBus.cpp
  src/Music.cpp
  src/ParticleController.cpp
  src/ParticleSystem.cpp
  src/PostBloom.cpp
  src/PostChromeAb.cpp
  src/PostProcess.cpp
  src/QuadTree.cpp
  src/QuadTreeComponent.cpp
  src/QuadTreeNode.cpp
  src/Reports.cpp
  src/Scene.cpp
  src/Score.cpp
  src/Server.cpp
  src/ShaderResource.cpp
  src/SoundPlayer.cpp
  src/State.cpp
  src/StateStack.cpp
  src/TextDrawable.cpp
  src/UIButton.cpp
  src/UICheckBox.cpp
  src/UIComboBox.cpp
  src/UIContainer.cpp
  src/UIControl.cpp
  src/UILabel.cpp
  src/UIScoreList.cpp
  src/UISelection.cpp
  src/UISlider.cpp
  src/UITextBox.cpp
  src/UIWindow.cpp)
  
if(WIN32)
  add_library(xygine STATIC WIN32 ${SOURCE})
else()
  add_library(xygine STATIC ${SOURCE})
endif()

target_link_libraries(xygine
  ${SFML_LIBRARIES}
  ${SFML_DEPENDENCIES})

if(UNIX)
  target_link_libraries(xygine
    ${X11_LIBRARIES})
endif()
