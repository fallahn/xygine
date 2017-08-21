#rename your project here
project(xyginext)
SET(PROJECT_NAME xyginext)
cmake_minimum_required(VERSION 2.8)

if(NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build (Debug or Release)" FORCE)
endif()

SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")

if(CMAKE_COMPILER_IS_GNUCXX)
  if(PROJECT_STATIC_RUNTIME)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++14 -static")
  else()
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++14")
  endif()
endif()

SET (CMAKE_CXX_FLAGS_DEBUG "-g -D_DEBUG_")
SET (CMAKE_CXX_FLAGS_RELEASE "-O4 -DNDEBUG")
SET (CMAKE_DEBUG_POSTFIX -d)

SET(SFML_MIN_VERSION "2.4")
if(WIN32)
  find_package(SFML ${SFML_MIN_VERSION} REQUIRED graphics window audio system network main)
else()
  find_package(SFML ${SFML_MIN_VERSION} REQUIRED graphics window audio system network)
endif()

if(UNIX)
  find_package(X11 REQUIRED)
endif()
find_package(OpenGL REQUIRED)
find_package(ENet REQUIRED)

include_directories(
  ${SFML_INCLUDE_DIR} 
  ${ENET_INCLUDE_DIR}
  ${CMAKE_SOURCE_DIR}/include)
  

SET(PROJECT_DIR ${CMAKE_SOURCE_DIR}/src)
include(${PROJECT_DIR}/CMakeLists.txt)

if(PROJECT_STATIC_RUNTIME)
  add_library(${PROJECT_NAME} STATIC ${PROJECT_SRC})
else()
  add_library(${PROJECT_NAME} SHARED ${PROJECT_SRC})
endif()


target_link_libraries(${PROJECT_NAME}
  ${SFML_LIBRARIES}
  ${SFML_DEPENDENCIES}
  ${ENET_LIBRARY}
  ${OPENGL_LIBRARIES})


target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/xyginext DESTINATION include)
if(XYGINEXT_STATIC_LIB)
  install(TARGETS ${PROJECT_NAME} EXPORT xyginext-targets DESTINATION lib)
else()
  install(TARGETS ${PROJECT_NAME} EXPORT xyginext-targets 
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin)
endif()
