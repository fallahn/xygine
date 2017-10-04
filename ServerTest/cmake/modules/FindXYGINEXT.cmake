include(FindPackageHandleStandardArgs)

# Search for the header file
find_path(XYXT_INCLUDE_DIR xyginext/Config.hpp PATH_SUFFIXES include)

# Search for the library
find_library(XYXT_LIBRARIES NAMES xyginext PATH_SUFFIXES lib)

# Did we find everything we need?
FIND_PACKAGE_HANDLE_STANDARD_ARGS(xyginext DEFAULT_MSG XYXT_LIBRARIES XYXT_INCLUDE_DIR) 
