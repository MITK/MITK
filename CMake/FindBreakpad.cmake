# Attempts to find a build directory of the Breakpad project (as provided by the MITK superbuild)
#
# Fills the following variables:
#  Breakpad_FOUND
#  Breakpad_INCLUDE_DIR
#  Breakpad_LIBRARIES
#

function(checkWindowsCompilerFlags)
  # TODO simplify this with foreach and some string functions
  set (WINDOWS_CXX_FLAGS_OK_ZI 0)
  if ( CMAKE_CXX_FLAGS_DEBUG MATCHES .*/Zi.* )
    set(WINDOWS_CXX_FLAGS_OK_ZI 1)
  endif()
  if ( CMAKE_CXX_FLAGS_RELEASE MATCHES .*/Zi.* )
    set(WINDOWS_CXX_FLAGS_OK_ZI 1)
  endif()
  if ( CMAKE_CXX_FLAGS MATCHES .*/Zi.* )
    set(WINDOWS_CXX_FLAGS_OK_ZI 1)
  endif()

  set (WINDOWS_C_FLAGS_OK_ZI 0)
  if ( CMAKE_C_FLAGS_DEBUG MATCHES .*/Zi.* )
    set(WINDOWS_C_FLAGS_OK_ZI 1)
  endif()
  if ( CMAKE_C_FLAGS_RELEASE MATCHES .*/Zi.* )
    set(WINDOWS_C_FLAGS_OK_ZI 1)
  endif()
  if ( CMAKE_C_FLAGS MATCHES .*/Zi.* )
    set(WINDOWS_C_FLAGS_OK_ZI 1)
  endif()

  if (NOT WINDOWS_CXX_FLAGS_OK_ZI)
    message(WARNING "When using the breakpad crash reporting module, you should use the /Zi flag in CMAKE_CXX_FLAGS (_RELEASE)")
  endif()
  if (NOT WINDOWS_C_FLAGS_OK_ZI)
    message(WARNING "When using the breakpad crash reporting module, you should use the /Zi flag in CMAKE_C_FLAGS (_RELEASE)")
  endif()

endfunction()

function(checkLinuxCompilerFlags)
  # TODO simplify this with foreach and some string functions
  set (LINUX_CXX_FLAGS_OK_G 0)
  if ( CMAKE_CXX_FLAGS_DEBUG MATCHES .*-g.* )
    set(LINUX_CXX_FLAGS_OK_G 1)
  endif()
  if ( CMAKE_CXX_FLAGS_RELEASE MATCHES .*-g.* )
    set(LINUX_CXX_FLAGS_OK_G 1)
  endif()
  if ( CMAKE_CXX_FLAGS MATCHES .*-g.* )
    set(LINUX_CXX_FLAGS_OK_G 1)
  endif()

  set (LINUX_C_FLAGS_OK_G 0)
  if ( CMAKE_C_FLAGS_DEBUG MATCHES .*-g.* )
    set(LINUX_C_FLAGS_OK_G 1)
  endif()
  if ( CMAKE_C_FLAGS_RELEASE MATCHES .*-g.* )
    set(LINUX_C_FLAGS_OK_G 1)
  endif()
  if ( CMAKE_C_FLAGS MATCHES .*-g.* )
    set(LINUX_C_FLAGS_OK_G 1)
  endif()

  if (NOT LINUX_CXX_FLAGS_OK_G)
    message(WARNING "When using the breakpad crash reporting module, you should use the -g flag in CMAKE_CXX_FLAGS (_RELEASE)")
  endif()
  if (NOT LINUX_C_FLAGS_OK_G)
    message(WARNING "When using the breakpad crash reporting module, you should use the -g flag in CMAKE_C_FLAGS (_RELEASE)")
  endif()


endfunction()



# -------------------- end functions ----------------------------

find_path(Breakpad_INCLUDE_DIR breakpad_googletest_includes.h DOC  "Directory breakpad/src/" PATHS ${Breakpad_SRC}/src ${Breakpad_DIR})

if(CMAKE_SYSTEM MATCHES "Windows")

  message(STATUS "Checking Windows build requirements for breakpad")

  checkWindowsCompilerFlags() # without debug information, Breakpad makes not much sense, so inform developer

  find_library(Breakpad_CLIENT_LIB crash_generation_client PATHS ${Breakpad_SRC}/src ${Breakpad_DIR} PATH_SUFFIXES Release Debug)
  find_library(Breakpad_SERVER_LIB crash_generation_server PATHS ${Breakpad_SRC}/src ${Breakpad_DIR} PATH_SUFFIXES Release Debug)
  find_library(Breakpad_EXCEPTIONHANDLER_LIB exception_handler PATHS ${Breakpad_SRC}/src ${Breakpad_DIR} PATH_SUFFIXES Release Debug)
  find_library(Breakpad_COMMON_LIB common PATHS ${Breakpad_SRC}/src ${Breakpad_DIR} PATH_SUFFIXES Release Debug)

  set(Breakpad_LIBRARIES
      ${Breakpad_CLIENT_LIB}
      ${Breakpad_SERVER_LIB}
      ${Breakpad_EXCEPTIONHANDLER_LIB}
      ${Breakpad_COMMON_LIB}
     )

elseif(CMAKE_SYSTEM MATCHES "Linux")

  message(STATUS "Checking Linux build requirements for breakpad")

  checkLinuxCompilerFlags() # without debug information, Breakpad makes not much sense, so inform developer

  find_library(Breakpad_CLIENT_LIB breakpad_client PATHS ${Breakpad_SRC}/src ${Breakpad_DIR})

  set(Breakpad_LIBRARIES
      ${Breakpad_CLIENT_LIB}
     )

else()
  message(FATAL_ERROR "Unsupported platform for Breakpad crash reporting: ${CMAKE_SYSTEM}")
endif()

if (Breakpad_LIBRARIES MATCHES "NOTFOUND")
  set(Breakpad_FOUND FALSE)
elseif(Breakpad_INCLUDE_DIR MATCHES "NOTFOUND")
  set(Breakpad_FOUND FALSE)
else()
  set(Breakpad_FOUND TRUE)
endif()

#message(STATUS "FindBreakpad... found it: ${Breakpad_FOUND}")
#message(STATUS " .. include at ${Breakpad_INCLUDE_DIR}")
#message(STATUS " .. link libraries ${Breakpad_LIBRARIES}")
