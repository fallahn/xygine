cmake_minimum_required(VERSION 3.1)

# Rename this variable to change the project name
SET(PROJECT_NAME xy_demo)

# Set up the project
project(${PROJECT_NAME})

# Set the path to our find modules
SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")

# Some default variables which the user may change
SET(CMAKE_BUILD_TYPE        Debug CACHE STRING  "Choose the type of build (Debug or Release)")
SET(MACOS_BUNDLE            False CACHE BOOL    "True to generate a macOS bundle (experimental)")

# We're using c++14
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# enable some warnings in debug builds with gcc/clang
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -Wall -Wextra -Wreorder")
elseif (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -Wall -Wextra -Wreorder -Wheader-guard")
endif()

# Only works with SFML version 2.4 and above
SET(SFML_MIN_VERSION 2.4)
find_package(SFML ${SFML_MIN_VERSION} REQUIRED graphics window audio system)

# Find xyginext and tmxlite
find_package(XYGINEXT REQUIRED)
find_package(TMXLITE REQUIRED)

# X11 is required on unices
if(UNIX AND NOT APPLE)
  find_package(X11 REQUIRED)
endif()

# Additional include directories
include_directories(
  ${XYXT_INCLUDE_DIR}
  ${SFML_INCLUDE_DIR} 
  ${TMXLITE_INCLUDE_DIR}
  ${CMAKE_SOURCE_DIR}/include)

if(X11_FOUND)
  include_directories(${X11_INCLUDE_DIRS})
endif()

# Project source files
add_subdirectory(src)

# Add XY_DEBUG on Debug builds
if (CMAKE_BUILD_TYPE MATCHES Debug) 
  add_definitions(-DXY_DEBUG)
endif()

# If on apple, create a nice bundle
If (APPLE AND MACOS_BUNDLE)
  set_source_files_properties( 
    ${CMAKE_SOURCE_DIR}/assets PROPERTIES MACOSX_PACKAGE_LOCATION Resources )
set_source_files_properties(
    ${CMAKE_SOURCE_DIR}/macOS/icon.icns PROPERTIES MACOSX_PACKAGE_LOCATION Resources )
  add_executable(${PROJECT_NAME} MACOSX_BUNDLE ${PROJECT_SRC} ${CMAKE_SOURCE_DIR}/assets ${CMAKE_SOURCE_DIR}/macOS/icon.icns)
  set_target_properties(${PROJECT_NAME} PROPERTIES MACOSX_BUNDLE_ICON_FILE icon.icns)
else()
  add_executable(${PROJECT_NAME} ${PROJECT_SRC})
endif()

# Linker settings
target_link_libraries(${PROJECT_NAME}
  ${SFML_LIBRARIES}
  ${SFML_DEPENDENCIES}
  ${TMXLITE_LIBRARIES}
  ${XYXT_LIBRARIES})

if(X11_FOUND)
  target_link_libraries(${PROJECT_NAME}
    ${X11_LIBRARIES})
endif()

# Install executable
install(TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION .
  BUNDLE DESTINATION .)

# Install assets folder except on Apple, where it's included in the app bundle
if (NOT (APPLE AND MACOS_BUNDLE))
  install(DIRECTORY assets
  DESTINATION .)
endif()

#Platform specific packaging
if(APPLE AND MACOS_BUNDLE)
	#set(CPACK_PACKAGE_FILE_NAME ${PROJECT_NAME})
    install(CODE " include(BundleUtilities)
		 fixup_bundle(${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/${PROJECT_NAME}.app \"\" \"\")
		 verify_app(${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/${PROJECT_NAME}.app)")
    SET(CPACK_GENERATOR "DragNDrop")
endif()

include(CPack)
