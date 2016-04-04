project(xygine)
cmake_minimum_required(VERSION 2.8)

if(NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build (Debug or Release)" FORCE)
endif()

if(NOT XY_STATIC_LIB)
  SET(XY_STATIC_LIB False CACHE BOOL "Should xygine be built as a static or shared lib")
endif()

SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")
SET(XY_STATIC_SFML FALSE CACHE BOOL "Choose whether SFML is linked statically or not.")
SET(XY_STATIC_RUNTIME FALSE CACHE BOOL "Use statically linked standard/runtime libraries? This option must match the one used for SFML.")

if(CMAKE_COMPILER_IS_GNUCXX)
  if(XY_STATIC_RUNTIME)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++1y -static")
  else()
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++1y -fPIC")
  endif()
endif()

if(XY_STATIC_LIB)
  SET(CMAKE_CXX_FLAGS_DEBUG "-g -D_DEBUG_ -DXY_STATIC")
  SET(CMAKE_CXX_FLAGS_RELEASE "-O4 -DNDEBUG -DXY_STATIC")
  SET(CMAKE_DEBUG_POSTFIX -s-d)
  SET(CMAKE_RELEASE_POSTFIX -s)
else()
  SET(CMAKE_CXX_FLAGS_DEBUG "-g -D_DEBUG_")
  SET(CMAKE_CXX_FLAGS_RELEASE "-O4 -DNDEBUG")
  SET(CMAKE_DEBUG_POSTFIX -d)
endif()

if(XY_STATIC_SFML)
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
find_package(OpenGL REQUIRED)

include_directories(
  ${SFML_INCLUDE_DIR}
  ${BOX2D_INCLUDE_DIR}
  ${OPENGL_INCLUDE_DIRECTORIES}
  ${CMAKE_SOURCE_DIR}/include)

if(X11_FOUND)
  include_directories(${X11_INCLUDE_DIRS})
endif()


SET(XY_DIR "src")
include("src/CMakeLists.txt")

  
if(WIN32)
  if(XY_STATIC_LIB)
    add_library(xygine STATIC WIN32 ${XY_SRC})
  else()
    add_library(xygine SHARED WIN32 ${XY_SRC})
  endif()   
else()
  if(XY_STATIC_LIB)
    add_library(xygine STATIC ${XY_SRC})
  else()
    add_library(xygine SHARED ${XY_SRC})
  endif()
endif()

target_link_libraries(xygine
  ${SFML_LIBRARIES}
  ${SFML_DEPENDENCIES}
  ${BOX2D_LIBRARIES}
  ${OPENGL_LIBRARIES})

if(UNIX)
  target_link_libraries(xygine
    ${X11_LIBRARIES})
endif()

#install xygine
install(DIRECTORY ${CMAKE_SOURCE_DIR}/include/xygine DESTINATION include)
if(XY_STATIC_LIB)
  install(TARGETS xygine EXPORT xygine-targets DESTINATION lib)
else()
  install(TARGETS xygine EXPORT xygine-targets 
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin)
endif()
