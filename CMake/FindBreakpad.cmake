
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

  set (WINDOWS_CXX_FLAGS_OK_DEBUG 0)
  if ( CMAKE_CXX_FLAGS_DEBUG MATCHES .*/debug.* )
    set(WINDOWS_CXX_FLAGS_OK_DEBUG 1)
  endif()
  if ( CMAKE_CXX_FLAGS_RELEASE MATCHES .*/debug.* )
    set(WINDOWS_CXX_FLAGS_OK_DEBUG 1)
  endif()
  if ( CMAKE_CXX_FLAGS MATCHES .*/debug.* )
    set(WINDOWS_CXX_FLAGS_OK_DEBUG 1)
  endif()

  set (WINDOWS_C_FLAGS_OK_DEBUG 0)
  if ( CMAKE_C_FLAGS_DEBUG MATCHES .*/debug.* )
    set(WINDOWS_C_FLAGS_OK_DEBUG 1)
  endif()
  if ( CMAKE_C_FLAGS_RELEASE MATCHES .*/debug.* )
    set(WINDOWS_C_FLAGS_OK_DEBUG 1)
  endif()
  if ( CMAKE_C_FLAGS MATCHES .*/debug.* )
    set(WINDOWS_C_FLAGS_OK_DEBUG 1)
  endif()

  if (NOT WINDOWS_CXX_FLAGS_OK_ZI)
    message(WARNING "When using the breakpad crash reporting module, you should use the /Zi flag in CMAKE_CXX_FLAGS (_RELEASE)")
  endif()
  if (NOT WINDOWS_C_FLAGS_OK_ZI)
    message(WARNING "When using the breakpad crash reporting module, you should use the /Zi flag in CMAKE_C_FLAGS (_RELEASE)")
  endif()

  if (NOT WINDOWS_CXX_FLAGS_OK_DEBUG)
    message(WARNING "When using the breakpad crash reporting module, you should use the /debug flag in CMAKE_CXX_FLAGS (_RELEASE)")
  endif()
  if (NOT WINDOWS_C_FLAGS_OK_DEBUG)
    message(WARNING "When using the breakpad crash reporting module, you should use the /debug flag in CMAKE_C_FLAGS (_RELEASE)")
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

  checkWindowsCompilerFlags() # without debug information, Breakpad makes not much sense, so inform developer

  find_library(Breakpad_CLIENT_LIB breakpad_client PATHS ${Breakpad_SRC}/src ${Breakpad_DIR} PATH_SUFFIXES Release Debug)

  set(Breakpad_LIBRARIES
      ${Breakpad_CLIENT_LIB}
     )

elseif(CMAKE_SYSTEM MATCHES "Linux")

  checkLinuxCompilerFlags() # without debug information, Breakpad makes not much sense, so inform developer

  find_library(Breakpad_CLIENT_LIB breakpad_client PATHS ${Breakpad_SRC}/src ${Breakpad_DIR})

  set(Breakpad_LIBRARIES
      ${Breakpad_CLIENT_LIB}
     )

else()
  message(FATAL_ERROR "Unsupported platform for Breakpad crash reporting: ${CMAKE_SYSTEM}")
endif()

message(STATUS "FindBreakpad...")
message(STATUS " .. include at ${Breakpad_INCLUDE_DIR}")
message(STATUS " .. link libraries ${Breakpad_LIBRARIES}")
