cmake_minimum_required(VERSION 3.1)

# Rename this variable to change the project name
SET(PROJECT_NAME xyginext)

# Set the path to our find modules
SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")

# Some default variables which the user may change
SET(CMAKE_BUILD_TYPE Debug CACHE STRING  "Choose the type of build (Debug or Release)")

# We're using c++14
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Only works with SFML version 2.4 and above
SET(SFML_MIN_VERSION 2.4)
find_package(SFML ${SFML_MIN_VERSION} REQUIRED graphics window audio system network)

# Also require OpenGL and ENet
find_package(OpenGL REQUIRED)
find_package(ENet REQUIRED)

# Additional include directories
include_directories(
  ${SFML_INCLUDE_DIR} 
  ${ENET_INCLUDE_DIR}
  ${CMAKE_SOURCE_DIR}/include)
  
# Project source files
add_subdirectory(src)

# Create the library
add_library(${PROJECT_NAME} ${PROJECT_SRC})

# Linker settings
target_link_libraries(${PROJECT_NAME}
  ${SFML_LIBRARIES}
  ${SFML_DEPENDENCIES}
  ${ENET_LIBRARIES}
  ${OPENGL_LIBRARIES})

# The project specific includes (Useful if linking to this target from another cmake script)
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Install the includes
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/xyginext DESTINATION include)

# And the rest
install(TARGETS ${PROJECT_NAME} EXPORT xyginext-targets 
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  RUNTIME DESTINATION bin)

 # Configuration for packaging
 include(CPack)

