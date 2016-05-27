project(xygine_example)
cmake_minimum_required(VERSION 2.8)

if(NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build (Debug or Release)" FORCE)
endif()

SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")
SET(xygine_example_STATIC_SFML FALSE CACHE BOOL "Choose whether SFML is linked statically or not. This must match Box2D linkage")
SET(xygine_example_STATIC_RUNTIME FALSE CACHE BOOL "Use statically linked standard/runtime libraries? This option must match the one used for SFML.")


if(CMAKE_COMPILER_IS_GNUCXX)
  if(xygine_example_STATIC_RUNTIME)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++14 -static")
  else()
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++14")
  endif()
endif()

SET (CMAKE_CXX_FLAGS_DEBUG "-g -D_DEBUG_ -DXY_STATIC")
SET (CMAKE_CXX_FLAGS_RELEASE "-O4 -DNDEBUG -DXY_STATIC")

if(xygine_example_STATIC_SFML)
  SET(SFML_STATIC_LIBRARIES TRUE)
endif()

SET(SFML_MIN_VERSION "2.3.2")
if(WIN32)
  find_package(SFML ${SFML_MIN_VERSION} REQUIRED graphics window audio system network main)
else()
  find_package(SFML ${SFML_MIN_VERSION} REQUIRED graphics window audio system network)
endif()

if(UNIX)
  find_package(X11 REQUIRED)
endif()

SET(BOX2D_MIN_VERSION "2.3.2")
find_package(BOX2D ${BOX2D_MIN_VERSION} REQUIRED)
find_package(XYGINE REQUIRED)

include_directories(
  ${XY_INCLUDE_DIR}
  ${SFML_INCLUDE_DIR}
  ${BOX2D_INCLUDE_DIR}  
  ${CMAKE_SOURCE_DIR}/include)

if(X11_FOUND)
  include_directories(${X11_INCLUDE_DIRS})
endif()


  include(${CMAKE_SOURCE_DIR}/src/CMakeLists.txt)

if(WIN32)
  add_executable(xygine_example WIN32 ${EXAMPLE_SRC})
else()
  add_executable(xygine_example ${EXAMPLE_SRC})
endif()

target_link_libraries(xygine_example
  ${SFML_LIBRARIES}
  ${SFML_DEPENDENCIES}
  ${BOX2D_LIBRARIES}
  ${XY_LIBRARIES})

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
