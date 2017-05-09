#.rst:
# FindWlc
# -------
#
# Find wlc library
#
# Try to find wlc library. The following values are defined
#
# ::
#
#   WLC_FOUND         - True if wlc is available
#   WLC_INCLUDE_DIRS  - Include directories for wlc
#   WLC_LIBRARIES     - List of libraries for wlc
#   WLC_DEFINITIONS   - List of definitions for wlc
#
#=============================================================================
# Copyright (c) 2015 Jari Vetoniemi
#
# Distributed under the OSI-approved BSD License (the "License");
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================

include(FeatureSummary)
set_package_properties(ewlc PROPERTIES
   URL "https://github.com/Enerccio/ewlc/"
   DESCRIPTION "Wayland compositor library - extended")

find_package(PkgConfig)
pkg_check_modules(PC_WLC QUIET ewlc)
find_path(WLC_INCLUDE_DIRS NAMES wlc/wlc.h HINTS ${PC_WLC_INCLUDE_DIRS})
find_library(WLC_LIBRARIES NAMES ewlc HINTS ${PC_WLC_LIBRARY_DIRS})

set(WLC_DEFINITIONS ${PC_WLC_CFLAGS_OTHER})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(wlc DEFAULT_MSG WLC_LIBRARIES WLC_INCLUDE_DIRS)
mark_as_advanced(WLC_LIBRARIES WLC_INCLUDE_DIRS WLC_DEFINITIONS)
