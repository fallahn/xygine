include(FindPackageHandleStandardArgs)

# Search for the header file
find_path(ENET_INCLUDE_DIR enet/enet.h PATH_SUFFIXES include)

# Search for the library
find_library(ENET_LIBRARY NAMES enet PATH_SUFFIXES lib)

# Did we find everything we need?
FIND_PACKAGE_HANDLE_STANDARD_ARGS(ENet DEFAULT_MSG ENET_LIBRARY ENET_INCLUDE_DIR) 

IF (ENET_FOUND)
    IF(WIN32)
        SET(WINDOWS_ENET_DEPENDENCIES "ws2_32;winmm")
        SET(ENET_LIBRARY ${ENET_LIBRARY} ${WINDOWS_ENET_DEPENDENCIES})
    ELSE
        SET(ENET_LIBRARY ${ENET_LIBRARY})
    ENDIF
ENDIF (ENET_FOUND)
