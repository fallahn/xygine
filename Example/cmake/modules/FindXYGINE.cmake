include(FindPackageHandleStandardArgs)

# Search for the header file
find_path(XY_INCLUDE_DIR xygine/Config.hpp PATH_SUFFIXES include)

# Search for the library
find_library(XY_LIBRARIES NAMES xygine PATH_SUFFIXES lib)

# Did we find everything we need?
FIND_PACKAGE_HANDLE_STANDARD_ARGS(xygine DEFAULT_MSG XY_LIBRARIES XY_INCLUDE_DIR) 
