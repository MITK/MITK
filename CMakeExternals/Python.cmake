#----------------------------------------------------------------------
# Python
#----------------------------------------------------------------------
if( MITK_USE_Python AND NOT MITK_USE_SYSTEM_PYTHON )
    # Sanity checks
  if(DEFINED Python_DIR AND NOT EXISTS ${Python_DIR})
    message(FATAL_ERROR "Python_DIR variable is defined but corresponds to non-existing directory")
  endif()

  if(NOT DEFINED Python_DIR)
    set(proj Python)
    set(proj_DEPENDENCIES )
    set(Python_DEPENDS ${proj})

    set(PYTHON_MAJOR_VERSION 2)
    set(PYTHON_MINOR_VERSION 7)
    set(PYTHON_PATCH_VERSION 3)

    set(PYTHON_SOURCE_PACKAGE Python-${PYTHON_MAJOR_VERSION}.${PYTHON_MINOR_VERSION}.${PYTHON_PATCH_VERSION})
    set(PYTHON_SOURCE_DIR  "${CMAKE_BINARY_DIR}/${PYTHON_SOURCE_PACKAGE}")
    # patch the VS compiler config

    if(WIN32)
      set(PYTHON_PATCH_COMMAND PATCH_COMMAND ${CMAKE_COMMAND} -DPYTHON_SOURCE_DIR:PATH=${PYTHON_SOURCE_DIR} -P ${CMAKE_CURRENT_LIST_DIR}/Patch${proj}.cmake)
    endif()

    # download the source code
    ExternalProject_Add(Python-src
      URL "https://dl.dropboxusercontent.com/u/8367205/ExternalProjects/${PYTHON_SOURCE_PACKAGE}.tgz"
      URL_MD5  "2cf641732ac23b18d139be077bd906cd"
      PREFIX   ${CMAKE_BINARY_DIR}/${PYTHON_SOURCE_PACKAGE}-cmake
      SOURCE_DIR  "${PYTHON_SOURCE_DIR}"
      ${PYTHON_PATCH_COMMAND}
      CONFIGURE_COMMAND ""
      BUILD_COMMAND ""
      INSTALL_COMMAND ""
    )

    set(additional_cmake_cache_args )
    list(APPEND additional_cmake_cache_args
        -DBUILTIN_ARRAY:BOOL=ON
        -DBUILTIN_AUDIOOP:BOOL=ON
        -DBUILTIN_BINASCII:BOOL=ON
        -DBUILTIN_BISECT:BOOL=ON
        -DBUILTIN_BSDB:BOOL=ON
        -DBUILTIN_BSSDB:BOOL=ON
        -DBUILTIN_BZ2:BOOL=ON
        -DBUILTIN_CMATH:BOOL=ON
        -DBUILTIN_COLLECTIONS:BOOL=ON
        -DBUILTIN_CODECS_CN:BOOL=ON
        -DBUILTIN_CODECS_HK:BOOL=ON
        -DBUILTIN_CODECS_ISO2022:BOOL=ON
        -DBUILTIN_CODECS_JP:BOOL=ON
        -DBUILTIN_CODECS_KR:BOOL=ON
        -DBUILTIN_CODECS_TW:BOOL=ON
        -DBUILTIN_CPICKLE:BOOL=ON
        -DBUILTIN_CRYPT:BOOL=ON
        -DBUILTIN_CSTRINGIO:BOOL=ON
        -DBUILTIN_CSV:BOOL=ON
        -DBUILTIN_CTYPES:BOOL=OFF
        #-DBUILTIN_CTYPES_TEST:BOOL=OFF
        #-DBUILTIN_CURSES:BOOL=ON
        -DBUILTIN_DATETIME:BOOL=ON
        -DBUILTIN_DBM:BOOL=ON
        -DBUILTIN_ELEMENTTREE:BOOL=ON
        -DBUILTIN_FCNTL:BOOL=ON
        -DBUILTIN_FUNCTOOLS:BOOL=ON
        -DBUILTIN_FUTURE_BUILTINS:BOOL=ON
        -DBULTIN_GDBM:BOOL=ON
        -DBUILTIN_GRP:BOOL=ON
        -DBUILTIN_HASHLIB:BOOL=ON
        -DBUILTIN_HEAPQ:BOOL=ON
        -DBUILTIN_HOTSHOT:BOOL=ON
        -DBUILTIN_IO:BOOL=ON
        -DBUILTIN_ITERTOOLS:BOOL=ON
        -DBUILTIN_JSON:BOOL=ON
        -DBUILTIN_LOCALE:BOOL=ON
        -DBUILTIN_LSPROF:BOOL=ON
        -DBUILTIN_MATH:BOOL=ON
        -DBUILTIN_MMAP:BOOL=ON
        -DBUILTIN_MULTIBYTECODEC:BOOL=ON
        -DBUILTIN_MD5:BOOL=ON
        -DBUILTIN_MULTIPROCESSING:BOOL=ON
        -DBUILTIN_NIS:BOOL=ON
        -DBUILTIN_NIT:BOOL=ON
        -DBUILTIN_OPERATOR:BOOL=ON
        -DBUILTIN_PARSER:BOOL=ON
        -DBUILTIN_POSIX:BOOL=ON
        -DBUILTIN_PWD:BOOL=ON
        -DBUILTIN_PYEXPAT:BOOL=ON
        -DBUILTIN_READLINE:BOOL=ON
        -DBUILTIN_RESOURCE:BOOL=ON
        -DBULTIN_RANDOM:BOOL=ON
        -DBUILTIN_SCPROXY:BOOL=OFF
        -DBUILTIN_SELECT:BOOL=ON
        -DBUILTIN_SHA:BOOL=ON
        -DBUILTIN_SHA256:BOOL=ON
        -DBUILTIN_SHA512:BOOL=ON
        -DBUILTIN_SOCKET:BOOL=ON
        -DBUILTIN_SPWD:BOOL=ON
        -DBUILTIN_SQLITE3:BOOL=OFF
        -DBUILTIN_SSL:BOOL=ON
        -DBUILTIN_STROP:BOOL=ON
        -DBUILTIN_STRUCT:BOOL=ON
        -DBUILTIN_SUBPROCESS:BOOL=ON
        -DBUILTIN_SYSLOG:BOOL=ON
        -DBUILTIN_TERMIOS:BOOL=ON
        #-DBUILTIN_TESTCAPI:BOOL=OFF
        -DBUILTIN_TIME:BOOL=ON
        -DBUILTIN_TKINTER:BOOL=ON
        -DBUILTIN_UNICODEDATA:BOOL=ON
        -DBUILTIN_WINREG:BOOL=ON
        -DBUILTIN_ZLIB:BOOL=ON
        )

    # CMake build environment for python
    ExternalProject_Add(${proj}
      URL "https://dl.dropboxusercontent.com/u/8367205/ExternalProjects/python-cmake-buildsystem.tar.gz"
      URL_MD5 "171090922892acdaf1a155e22765d72d"
      SOURCE_DIR ${CMAKE_BINARY_DIR}/${proj}-src
      PREFIX ${proj}-cmake
      BINARY_DIR ${proj}-build
      INSTALL_DIR ${proj}-install
      CMAKE_ARGS
        ${ep_common_args}
        -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
      CMAKE_CACHE_ARGS
        -DCMAKE_CXX_COMPILER:FILEPATH=${CMAKE_CXX_COMPILER}
        -DCMAKE_C_COMPILER:FILEPATH=${CMAKE_C_COMPILER}
        #-DBUILD_TESTING:BOOL=OFF
        -DBUILD_SHARED:BOOL=ON
        -DBUILD_STATIC:BOOL=OFF
        -DUSE_SYSTEM_LIBRARIES:BOOL=ON
        ${additional_cmake_cache_args}
      DEPENDS
        Python-src ${${proj}_DEPENDENCIES}
    )

    set(Python_DIR "${CMAKE_BINARY_DIR}/${proj}-install")
    set(Python_BUILD_DIR "${CMAKE_BINARY_DIR}/${proj}-build")

    # use the python executable in the build dir for unix systems. The stripped
    # ones will cause conflicts if system libraries are present during the build/configure process
    # of opencv, since they will try to lookup the sys path first if no lib is directly
    # linked with it s path into the executable
    if(UNIX)
      set(PYTHON_EXECUTABLE "${Python_BUILD_DIR}/bin/python${CMAKE_EXECUTABLE_SUFFIX}")
      set(PYTHON_INCLUDE_DIR "${Python_DIR}/include/python${PYTHON_MAJOR_VERSION}.${PYTHON_MINOR_VERSION}")
      set(PYTHON_LIBRARY "${Python_BUILD_DIR}/lib/${CMAKE_SHARED_LIBRARY_PREFIX}python${PYTHON_MAJOR_VERSION}.${PYTHON_MINOR_VERSION}${CMAKE_SHARED_LIBRARY_SUFFIX}")
      set(ENV{PYTHONHOME} "${Python_BUILD_DIR}")
    else()
      set(PYTHON_EXECUTABLE "${Python_DIR}/bin/python${CMAKE_EXECUTABLE_SUFFIX}")
      set(PYTHON_INCLUDE_DIR "${Python_DIR}/include")
      set(PYTHON_LIBRARY "${Python_DIR}/libs/python${PYTHON_MAJOR_VERSION}${PYTHON_MINOR_VERSION}.lib")
      set(ENV{PYTHONHOME} "${Python_DIR}")
    endif()

    # get the name of the library
    get_filename_component(PYTHON_LIBRARY_NAME "${PYTHON_LIBRARY}" NAME)

  else()
    mitkMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")
  endif()
endif()

