# FindBox2D.cmake
# Copyright Â© 2013 Matteo Cypriani
#
# This file is free software. It comes without any warranty, to the
# extent permitted by applicable law. You can redistribute it and/or
# modify it under the terms of the Do What The Fuck You Want To Public
# License, Version 2, as published by Sam Hocevar. See
# http://sam.zoy.org/wtfpl/COPYING for more details.

# This module tries to find the Box2D library and sets the following
# variables:
#   BOX2D_INCLUDE_DIR
#   BOX2D_LIBRARIES
#   BOX2D_FOUND

include(FindPackageHandleStandardArgs)

# Search for the header file
find_path(BOX2D_INCLUDE_DIR Box2D/Box2D.h
  PATH_SUFFIXES include)

# Search for the library
find_library(BOX2D_LIBRARIES NAMES Box2D
  PATH_SUFFIXES lib)

# Did we find everything we need?
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Box2D DEFAULT_MSG
  BOX2D_LIBRARIES BOX2D_INCLUDE_DIR)
