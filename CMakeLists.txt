cmake_minimum_required(VERSION 3.1)

# Rename this variable to change the project name
set(PROJECT_NAME xyginext)
set(DEMO_NAME xy-demo)

# Xcode 10 will default to Mojave (must be done BEFORE project())
if (APPLE)
 set(CMAKE_OSX_DEPLOYMENT_TARGET "10.13" CACHE STRING "Minimum OS X deployment version")
endif()

project(${PROJECT_NAME})

# Set the path to our find modules
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")

# Some default variables which the user may change
option(CMAKE_BUILD_TYPE "Choose the type of build (Debug or Release)" Debug)
option(BUILD_SHARED_LIBS "Whether to build shared libraries" ON)
option(BUILD_DEMO "Build the xygine demo" OFF)

# We're using c++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_DEBUG_POSTFIX -d)

# enable some warnings in debug builds with gcc/clang
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -Wall -Wextra -Wreorder")
elseif (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -Wall -Wextra -Wreorder -Wheader-guard")
endif()

# Apple requires some extra guff
if (APPLE)
  find_library(CORESERVICES_LIBRARY CoreServices)
  find_library(APPKIT AppKit)
elseif(UNIX)
  find_package(PkgConfig REQUIRED)
  pkg_check_modules(GTK REQUIRED gtk+-3.0)
endif()

# Only works with SFML version 2.5 and above
SET(SFML_MIN_VERSION 2.5)
find_package(SFML REQUIRED COMPONENTS graphics window audio system)

# Also require OpenGL and ENet
SET (OpenGL_GL_PREFERENCE "GLVND")
find_package(OpenGL REQUIRED)
find_package(ENet REQUIRED)

# xyginext source files
add_subdirectory(xyginext/src)
add_subdirectory(xyginext/include/xyginext)

# platform specific for file dialogue
if(APPLE)
  set(NFD_SRC 
    xyginext/src/core/dialogues/nfd/nfd_common.c
    xyginext/src/core/dialogues/nfd/nfd_cocoa.m)
elseif(UNIX)
  set(NFD_SRC 
    xyginext/src/core/dialogues/nfd/nfd_common.c
    xyginext/src/core/dialogues/nfd/nfd_gtk.c)
elseif(WIN32)
  set(NFD_SRC 
    xyginext/src/core/dialogues/nfd/nfd_common.c
    xyginext/src/core/dialogues/nfd/nfd_win.cpp)
endif()

# Add the Resourcepath source
if (APPLE)
  set(PROJECT_SRC ${PROJECT_SRC} xyginext/src/detail/ResourcePath.mm)
endif()

# Add XY_DEBUG on Debug builds
if (CMAKE_BUILD_TYPE MATCHES Debug) 
  add_definitions(-DXY_DEBUG)
endif()

if (NOT BUILD_SHARED_LIBS)
    add_definitions(-DSFML_STATIC -DXY_STATIC)
endif()

# Create the xyginext ibrary target
add_library(${PROJECT_NAME} ${PROJECT_SRC} ${NFD_SRC})

# Linker settings
if (BUILD_SHARED_LIBS)
target_link_libraries(${PROJECT_NAME}
  sfml-graphics
  sfml-audio
  sfml-system
  ${ENET_LIBRARIES}
  ${OPENGL_LIBRARIES})
else()
target_link_libraries(${PROJECT_NAME}
  sfml-graphics-s
  sfml-audio-s
  sfml-system-s
  ${ENET_LIBRARIES}
  ${OPENGL_LIBRARIES})
endif()

if (APPLE)
  target_link_libraries(${PROJECT_NAME} ${CORESERVICES_LIBRARY} ${APPKIT})
elseif(UNIX)
  target_link_libraries(${PROJECT_NAME} ${GTK_LIBRARIES})
endif()

# Additional include directories
target_include_directories(${PROJECT_NAME} PUBLIC 
              "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/xyginext/include>"
              "$<INSTALL_INTERFACE:include>")

target_include_directories( ${PROJECT_NAME} PRIVATE
  ${ENET_INCLUDE_DIR})
  
if(UNIX AND NOT APPLE)
  target_include_directories(${PROJECT_NAME} PRIVATE ${GTK_INCLUDE_DIRS})
endif()

# The demo target
if (BUILD_DEMO)
  add_subdirectory(Demo/src)
  if (APPLE)
    set_source_files_properties( 
    ${CMAKE_SOURCE_DIR}/Demo/assets PROPERTIES MACOSX_PACKAGE_LOCATION Resources )
    set_source_files_properties(
    ${CMAKE_SOURCE_DIR}/Demo/macOS/icon.icns PROPERTIES MACOSX_PACKAGE_LOCATION Resources )
    add_executable(${DEMO_NAME} MACOSX_BUNDLE ${DEMO_SRC} ${CMAKE_SOURCE_DIR}/Demo/assets ${CMAKE_SOURCE_DIR}/Demo/macOS/icon.icns)
    set_target_properties(${DEMO_NAME} PROPERTIES MACOSX_BUNDLE_ICON_FILE icon.icns)
  else()
    add_executable(${DEMO_NAME} ${DEMO_SRC})
  endif()
  add_dependencies(${DEMO_NAME} ${PROJECT_NAME})

  find_package(TMXLITE REQUIRED)

  if(UNIX AND NOT APPLE)
    find_package(X11 REQUIRED)
    target_link_libraries(${DEMO_NAME} ${X11_LIBRARIES})
  endif()

  if(BUILD_SHARED_LIBS)  
    target_link_libraries(${DEMO_NAME} ${PROJECT_NAME} ${TMXLITE_LIBRARIES} sfml-graphics sfml-audio)
  else()
    target_link_libraries(${DEMO_NAME} ${PROJECT_NAME} ${TMXLITE_LIBRARIES} sfml-graphics-s sfml-audio-s)
  endif()
  target_include_directories(${DEMO_NAME} PRIVATE ${TMXLITE_INCLUDE_DIR} $<TARGET_PROPERTY:${PROJECT_NAME},INTERFACE_INCLUDE_DIRECTORIES>)
  
  
  install(TARGETS ${DEMO_NAME} DESTINATION .)
endif()

# CMake package config setup
set(CONFIG_FILE "${CMAKE_CURRENT_SOURCE_DIR}/cmake/generated/${PROJECT_NAME}-config.cmake")
set(CONFIG_DEST "lib${LIB_SUFFIX}/cmake/${PROJECT_NAME}")
set(TARGETS_EXPORT_NAME "${PROJECT_NAME}Targets")

include(CMakePackageConfigHelpers)
configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/xygine-config.cmake.in"
    ${CONFIG_FILE}
    INSTALL_DESTINATION ${CONFIG_DEST}
)

# Install
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/xyginext/include/xyginext DESTINATION include)

install(TARGETS ${PROJECT_NAME} EXPORT ${TARGETS_EXPORT_NAME}
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib)

install(FILES ${CONFIG_FILE} DESTINATION ${CONFIG_DEST})

install( EXPORT ${TARGETS_EXPORT_NAME}
         DESTINATION "${CONFIG_DEST}")

# Version number
set(CPACK_PACKAGE_VERSION_MAJOR 0)
set(CPACK_PACKAGE_VERSION_MINOR 2)
set(CPACK_PACKAGE_VERSION_PATCH 0)

# Configuration for packaging
set(CPACK_GENERATOR "ZIP")
include(CPack)
