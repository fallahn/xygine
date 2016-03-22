project(xygine_example)
cmake_minimum_required(VERSION 2.8)

if(NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build (Debug or Release)" FORCE)
endif()

SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")
SET(xygine_example_STATIC_SFML FALSE CACHE BOOL "Choose whether SFML is linked statically or not.")
SET(xygine_example_STATIC_RUNTIME FALSE CACHE BOOL "Use statically linked standard/runtime libraries? This option must match the one used for SFML.")


if(CMAKE_COMPILER_IS_GNUCXX)
  if(xygine_example_STATIC_RUNTIME)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++1y -static")
  else()
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++1y")
  endif()
endif()

SET (CMAKE_CXX_FLAGS_DEBUG "-g -D_DEBUG_ -DXY_STATIC")
SET (CMAKE_CXX_FLAGS_RELEASE "-O4 -DNDEBUG -DXY_STATIC")

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

find_package(BOX2D REQUIRED)

include_directories(
  ${SFML_INCLUDE_DIR}
  ${BOX2D_INCLUDE_DIR}  
  ${CMAKE_SOURCE_DIR}/include
  ${CMAKE_SOURCE_DIR}/../xygine/include)

if(X11_FOUND)
  include_directories(${X11_INCLUDE_DIRS})
endif()


SET(XY_DIR ${CMAKE_SOURCE_DIR}/../xygine/src)
include(${XY_DIR}/CMakeLists.txt)

SET(EXAMPLE_SRC
  src/Game.cpp
  src/CaveDemoDrawable.cpp
  src/LMAlienController.cpp
  src/LMCollisionComponent.cpp
  src/LMCollisionWorld.cpp
  src/LMGameController.cpp
  src/LMHumanController.cpp
  src/LMMothershipController.cpp
  src/LMPlayerController.cpp
  src/LunarMoonerState.cpp
  src/main.cpp
  src/MenuMainState.cpp
  src/MenuOptionState.cpp
  src/NetworkDemoBallLogic.cpp
  src/NetworkDemoBot.cpp
  src/NetworkDemoCollisions.cpp
  src/NetworkDemoController.cpp
  src/NetworkDemoPlayerController.cpp
  src/NetworkDemoServer.cpp
  src/NetworkDemoState.cpp
  src/ParticleDemoState.cpp
  src/PhysicsDemoInputController.cpp
  src/PhysicsDemoLine.cpp
  src/PhysicsDemoState.cpp)

if(WIN32)
  add_executable(xygine_example WIN32 ${EXAMPLE_SRC} ${XY_SRC})
else()
  add_executable(xygine_example ${EXAMPLE_SRC} ${XY_SRC})
endif()

target_link_libraries(xygine_example
  ${SFML_LIBRARIES}
  ${SFML_DEPENDENCIES}
  ${BOX2D_LIBRARIES})

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
