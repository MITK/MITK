# - Find the Poco includes and libraries.
# The following variables are set if Poco is found.  If Poco is not
# found, Poco_FOUND is set to false.
#  Poco_FOUND        - True when the Poco include directory is found.
#  Poco_INCLUDE_DIRS - the path to where the poco include files are.
#  Poco_LIBRARY_DIRS - The path to where the poco library files are.
#  Poco_BINARY_DIRS - The path to where the poco dlls are.

# ----------------------------------------------------------------------------
# If you have installed Poco in a non-standard location.
# Then you have three options.
# In the following comments, it is assumed that <Your Path>
# points to the root directory of the include directory of Poco. e.g
# If you have put poco in C:\development\Poco then <Your Path> is
# "C:/development/Poco" and in this directory there will be two
# directories called "include" and "lib".
# 1) After CMake runs, set Poco_INCLUDE_DIR to <Your Path>/poco<-version>
# 2) Use CMAKE_INCLUDE_PATH to set a path to <Your Path>/poco<-version>. This will allow find_path()
#    to locate Poco_INCLUDE_DIR by utilizing the PATH_SUFFIXES option. e.g.
#    set(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} "<Your Path>/include")
# 3) Set an environment variable called ${POCO_ROOT} that points to the root of where you have
#    installed Poco, e.g. <Your Path>. It is assumed that there is at least a subdirectory called
#    Foundation/include/Poco in this path.
#
# Note:
#  1) If you are just using the poco headers, then you do not need to use
#     Poco_LIBRARY_DIR in your CMakeLists.txt file.
#  2) If Poco has not been installed, then when setting Poco_LIBRARY_DIR
#     the script will look for /lib first and, if this fails, then for /stage/lib.
#
# Usage:
# In your CMakeLists.txt file do something like this:
# ...
# # Poco
# find_package(Poco)
# ...
# include_directories(${Poco_INCLUDE_DIRS})
# link_directories(${Poco_LIBRARY_DIR})
#
# In Windows, we make the assumption that, if the Poco files are installed, the default directory
# will be C:\poco or C:\Program Files\Poco or C:\Programme\Poco.

set(POCO_INCLUDE_PATH_DESCRIPTION "top-level directory containing the poco include directories. E.g /usr/local/include/ or c:\\poco\\include\\poco-1.3.2")
set(POCO_INCLUDE_DIR_MESSAGE "Set the Poco_INCLUDE_DIR cmake cache entry to the ${POCO_INCLUDE_PATH_DESCRIPTION}")
set(POCO_LIBRARY_PATH_DESCRIPTION "directory containing the poco libraries:")


set(POCO_DIR_SEARCH $ENV{POCO_ROOT})
if(POCO_DIR_SEARCH)
  file(TO_CMAKE_PATH ${POCO_DIR_SEARCH} POCO_DIR_SEARCH)
endif(POCO_DIR_SEARCH)
list(APPEND POCO_DIR_SEARCH ${Poco_DIR})

if(WIN32)
  set(POCO_DIR_SEARCH
    ${POCO_DIR_SEARCH}
    C:/poco
    D:/poco
    "C:/Program Files/poco"
    "C:/Programme/poco"
    "D:/Program Files/poco"
    "D:/Programme/poco"
  )
endif(WIN32)

# Add in some path suffixes. These will have to be updated whenever a new Poco version comes out.
set(SUFFIX_FOR_INCLUDE_PATH
 include
)

set(SUFFIX_FOR_LIBRARY_PATH
 lib
 bin
)

#
# Look for an installation.
#
find_path(Poco_INCLUDE_DIR NAMES Poco/Poco.h PATH_SUFFIXES ${SUFFIX_FOR_INCLUDE_PATH} PATHS

  # Look in other places.
  ${POCO_DIR_SEARCH}

  # Help the user find it if we cannot.
  DOC "The ${POCO_INCLUDE_PATH_DESCRIPTION}"
)

if(NOT Poco_INCLUDE_DIR)

  # Look for standard unix include paths
  find_path(Poco_INCLUDE_DIR Poco/Poco.h DOC "The ${POCO_INCLUDE_PATH_DESCRIPTION}")

endif(NOT Poco_INCLUDE_DIR)

# Assume we didn't find it.
set(Poco_FOUND 0)

set(Poco_LIBRARIES )
set(Poco_LIBRARY_DIRS )

# Now try to get the include and library path.
# Find all libraries, store debug and release separately
foreach(lib
    Crypto
    Data
    DataMySQL
    DataSQLite
    Foundation
    JSON
    MongoDB
    Net
    NetSSL
    PDF
    Util
    XML
    Zip
    )

  # Find Release libraries
  find_library(Poco_${lib}_LIBRARY_RELEASE
    NAMES Poco${lib}
    PATH_SUFFIXES ${SUFFIX_FOR_LIBRARY_PATH}
    PATHS ${POCO_DIR_SEARCH}
    DOC "The ${POCO_LIBRARY_PATH_DESCRIPTION} Poco${lib}"
    )

  # Find Debug libraries
  find_library(Poco_${lib}_LIBRARY_DEBUG
    NAMES Poco${lib}d
    PATH_SUFFIXES ${SUFFIX_FOR_LIBRARY_PATH}
    PATHS ${POCO_DIR_SEARCH}
    DOC "The ${POCO_LIBRARY_PATH_DESCRIPTION} Poco${lib}d"
    )

  mark_as_advanced(Poco_${lib}_LIBRARY_RELEASE)
  mark_as_advanced(Poco_${lib}_LIBRARY_DEBUG)

  # Add libraries to variable according to build type
  set(Poco_${lib}_LIBRARY)
  if(Poco_${lib}_LIBRARY_RELEASE)
    list(APPEND Poco_LIBRARIES optimized ${Poco_${lib}_LIBRARY_RELEASE})
    list(APPEND Poco_${lib}_LIBRARY optimized ${Poco_${lib}_LIBRARY_RELEASE})
    get_filename_component(lib_dir ${Poco_${lib}_LIBRARY_RELEASE} PATH)
    list(APPEND Poco_LIBRARY_DIRS ${lib_dir})
  endif()

  if(Poco_${lib}_LIBRARY_DEBUG)
    list(APPEND Poco_LIBRARIES debug ${Poco_${lib}_LIBRARY_DEBUG})
    list(APPEND Poco_${lib}_LIBRARY debug ${Poco_${lib}_LIBRARY_DEBUG})
    get_filename_component(lib_dir ${Poco_${lib}_LIBRARY_RELEASE} PATH)
    list(APPEND Poco_LIBRARY_DIRS ${lib_dir})
  endif()

endforeach()

if(Poco_LIBRARY_DIRS)
  list(REMOVE_DUPLICATES Poco_LIBRARY_DIRS)
endif()

set(Poco_INCLUDE_DIRS ${Poco_INCLUDE_DIR})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Poco DEFAULT_MSG
  Poco_INCLUDE_DIRS
  Poco_Foundation_LIBRARY
  Poco_LIBRARIES
  )

