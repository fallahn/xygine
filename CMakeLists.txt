cmake_minimum_required(VERSION 3.11)

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

# CMake package config setup
set(CONFIG_FILE "${CMAKE_CURRENT_SOURCE_DIR}/cmake/generated/${PROJECT_NAME}-config.cmake")
set(CONFIG_DEST "lib${LIB_SUFFIX}/cmake/${PROJECT_NAME}")
set(TARGETS_EXPORT_NAME "${PROJECT_NAME}Targets")

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

# disable deprecated warning in MSVC
# enable correct import/export symbols in shared builds
if(MSVC)
  add_definitions(-D_CRT_SECURE_NO_WARNINGS)
  if(BUILD_SHARED_LIBS)
    add_definitions(-DBUILD_XY)
  endif()
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
find_package(SFML QUIET COMPONENTS graphics window audio system)

# If SFML is not found, get it from github
if (NOT SFML_FOUND)
  message(STATUS "SFML not found, fetching from github...")
  include(FetchContent)
  FetchContent_Declare(
    sfml
    GIT_REPOSITORY https://github.com/sfml/sfml.git
    # 2.5.1 actually uses auto_ptr(!) which is incompatible with 
    # modern c++ so we use a slightly later commit which fixes
    # this issue
    GIT_TAG        bf92efe9a4035fee0258386173d53556aa196e49
  )
  FetchContent_MakeAvailable(sfml)
endif()

# Also require OpenGL and ENet
SET (OpenGL_GL_PREFERENCE "GLVND")
find_package(OpenGL REQUIRED)
find_package(ENet QUIET)

# If ENet isn't found we can get it from github
if (NOT ENet_FOUND)
  message(STATUS "ENet not found, fetching from github...")
  include(FetchContent)
  FetchContent_Declare(
    enet
    GIT_REPOSITORY https://github.com/lsalzman/enet.git
    GIT_TAG        v1.3.16
  )
  FetchContent_MakeAvailable(enet)

  # ENet's cmake is unbelievably basic, so we have to set some stuff ourselves
  set(ENET_INCLUDE_DIR "${enet_SOURCE_DIR}/include")
  set(ENET_LIBRARIES enet)
  install(TARGETS enet EXPORT ${TARGETS_EXPORT_NAME})
  if (WIN32)
    list(APPEND ENET_LIBRARIES ws2_32 winmm)
  endif()
endif()

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
target_link_libraries(${PROJECT_NAME}
  sfml-graphics
  sfml-audio
  sfml-system
  ${ENET_LIBRARIES}
  ${OPENGL_LIBRARIES}
  ${CMAKE_DL_LIBS})

# debug output libs for visual studio
if (CMAKE_BUILD_TYPE MATCHES Debug AND MSVC) 
  target_link_libraries(${PROJECT_NAME}
    dgbhelp shlwapi)
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

  find_package(TMXLITE QUIET)

  if (NOT ${TMXLITE_FOUND})
    message(STATUS "tmxlite not found, fetching from github...")
    include(FetchContent)
    FetchContent_Declare(
      tmxlite
      GIT_REPOSITORY https://github.com/fallahn/tmxlite
      GIT_TAG        1.2.2
    )
    FetchContent_MakeAvailable(tmxlite)
    add_subdirectory(${tmxlite_SOURCE_DIR}/tmxlite)
    set(TMXLITE_LIBRARIES tmxlite)
    set(TMXLITE_INCLUDE_DIR ${tmxlite_SOURCE_DIR}/tmxlite/include)
  endif()

  if(UNIX AND NOT APPLE)
    find_package(X11 REQUIRED)
    target_link_libraries(${DEMO_NAME} ${X11_LIBRARIES})
  endif()

  target_link_libraries(${DEMO_NAME} ${PROJECT_NAME} ${TMXLITE_LIBRARIES} sfml-graphics sfml-audio)
  target_include_directories(${DEMO_NAME} PRIVATE ${TMXLITE_INCLUDE_DIR} $<TARGET_PROPERTY:${PROJECT_NAME},INTERFACE_INCLUDE_DIRECTORIES>)
  
  
  install(TARGETS ${DEMO_NAME} DESTINATION .)
endif()

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
