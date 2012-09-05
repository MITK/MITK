# Set MITK Workbench specific CPack options

# set version
set(CPACK_PACKAGE_VERSION
  "${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")

# determine possible system specific extension
set(extension "unkown-architecture")

if(${CMAKE_SYSTEM_NAME} MATCHES Windows)
  if(CMAKE_CL_64)
    set(extension "win64")
  elseif(MINGW)
    set(extension "mingw32")
  elseif(WIN32)
    set(extension "win32")
  endif()
endif(${CMAKE_SYSTEM_NAME} MATCHES Windows)

if(${CMAKE_SYSTEM_NAME} MATCHES Linux)
  if(${CMAKE_SYSTEM_PROCESSOR} MATCHES i686)
    set(extension "linux32")
  elseif(${CMAKE_SYSTEM_PROCESSOR} MATCHES x86_64)
    if(${CMAKE_CXX_FLAGS} MATCHES " -m32 ")
      set(extension "linux32")
    else()
      set(extension "linux64")
    endif(${CMAKE_CXX_FLAGS} MATCHES " -m32 ")
  else()
    set(extension "linux")
  endif()
endif(${CMAKE_SYSTEM_NAME} MATCHES Linux)

if(${CMAKE_SYSTEM_NAME} MATCHES Darwin)
  set(extension "mac64")
endif(${CMAKE_SYSTEM_NAME} MATCHES Darwin)

set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${extension}")
