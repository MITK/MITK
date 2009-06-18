PROJECT(poco)

cmake_minimum_required(VERSION 2.6)

set(CPACK_PACKAGE_VERSION_MAJOR "1")
set(CPACK_PACKAGE_VERSION_MINOR "3")
set(CPACK_PACKAGE_VERSION_PATCH "5")
set(COMPLETE_VERSION ${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH})
set(RELEASE_NAME "Poco 1.3.5")
set(PROJECT_VERSION ${COMPLETE_VERSION})

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "" CACHE PATH "The single directory for all runtime files")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "" CACHE PATH "The single directory for all library files")


#set(COMMON_CXX "-Wall -Wno-sign-compare")
set(CMAKE_CXX_FLAGS_DEBUG "${COMMON_CXX} ${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG")
set(CMAKE_CXX_FLAGS_MINSIZEREL "${COMMON_CXX} ${CMAKE_CXX_FLAGS_MINSIZEREL}")

# Set the library postfix
set(CMAKE_DEBUG_POSTFIX "d")

# Uncomment from next two lines to force statitc or dynamic library, default is autodetection
if(POCO_STATIC)
    set( LIB_MODE STATIC )
else(POCO_STATIC)
    set( LIB_MODE SHARED )
endif(POCO_STATIC)

# whether unit tests should be build
set (POCO_ENABLE_TESTS FALSE CACHE BOOL "Build the Poco unit tests")
  mark_as_advanced(POCO_ENABLE_TESTS)
set(ENABLE_TESTS ${POCO_ENABLE_TESTS})

IF (ENABLE_TESTS)
  ENABLE_TESTING()
ENDIF (ENABLE_TESTS)

# Set local include path
#include_directories( CppUnit/include Foundation/include XML/include Net/include NetSSL_OpenSSL/include Util/include Data/include WebWidgets/include Zip/include)
include_directories( CppUnit/include Foundation/include XML/include Util/include )

include(CheckTypeSize)
include(FindCygwin)
#include(FindOpenSSL)
include(CMakeDetermineCompilerId)

#include(contrib/cmake/FindMySQL.cmake)
#include(contrib/cmake/FindAPR.cmake)
#include(contrib/cmake/FindApache2.cmake)

# OS Detection
if(CMAKE_SYSTEM MATCHES "Windows")
 add_definitions( -DPOCO_OS_FAMILY_WINDOWS -DPOCO_WIN32_UTF8)
  set(SYSLIBS iphlpapi gdi32)
endif(CMAKE_SYSTEM MATCHES "Windows")

if (CMAKE_SYSTEM MATCHES "Linux")
  add_definitions( -DPOCO_OS_FAMILY_UNIX )
  # Standard 'must be' defines
  add_definitions( -D_XOPEN_SOURCE=500 -D_REENTRANT -D_THREAD_SAFE -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64)
  set(SYSLIBS  pthread dl rt)
endif(CMAKE_SYSTEM MATCHES "Linux")

if (CMAKE_SYSTEM MATCHES "SunOS")
  add_definitions( -DPOCO_OS_FAMILY_UNIX )
  # Standard 'must be' defines
  add_definitions( -D_XOPEN_SOURCE=500 -D_REENTRANT -D_THREAD_SAFE -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 )
  set(SYSLIBS  pthread socket xnet nsl resolv rt dl)
endif(CMAKE_SYSTEM MATCHES "SunOS")

if (CMAKE_COMPILER_IS_MINGW)
    add_definitions(-DWC_NO_BEST_FIT_CHARS=0x400)
    add_definitions(-mno-cygwin -D_WIN32 -DMINGW32 -DWINVER=0x500 -DODBCVER=0x0300 -DPOCO_THREAD_STACK_SIZE -DFoundation_Config_INCLUDED )
  #link_directories(/usr/local/lib /usr/lib)
  #include_directories(/usr/local/include /usr/include)
endif (CMAKE_COMPILER_IS_MINGW)

if (CMAKE_COMPILER_IS_CYGWIN)
#    add_definitions(-DWC_NO_BEST_FIT_CHARS=0x400)
endif (CMAKE_COMPILER_IS_CYGWIN)

# SunPro C++
if (${CMAKE_CXX_COMPILER_ID} MATCHES "SunPro")
  add_definitions( -D_BSD_SOURCE -library=stlport4)
endif (${CMAKE_CXX_COMPILER_ID} MATCHES "SunPro")

add_subdirectory(Foundation)
add_subdirectory(XML)
add_subdirectory(Util)
#add_subdirectory(Net)

#if(OPENSSL_FOUND)
#    add_subdirectory(NetSSL_OpenSSL)
#    add_subdirectory(Crypto)
#endif(OPENSSL_FOUND)

#add_subdirectory(Data)
#add_subdirectory(WebWidgets)
#add_subdirectory(Zip)

if (ENABLE_TESTS)
  add_subdirectory(CppUnit)
  add_subdirectory(Foundation/testsuite)
  add_subdirectory(XML/testsuite)
  add_subdirectory(Util/testsuite)
endif (ENABLE_TESTS)


# Enable packaging

INCLUDE(InstallRequiredSystemLibraries)

