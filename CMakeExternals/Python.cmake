#----------------------------------------------------------------------
# Python
#----------------------------------------------------------------------
if( MITK_USE_Python AND NOT MITK_USE_SYSTEM_PYTHON )
    # Sanity checks
  if(DEFINED Python_DIR AND NOT EXISTS ${Python_DIR})
    message(FATAL_ERROR "Python_DIR variable is defined but corresponds to non-existing directory")
  endif()

  set(MITK_PYTHON_MAJOR_VERSION 2)
  set(MITK_PYTHON_MINOR_VERSION 7)
  set(MITK_PYTHON_PATCH_VERSION 3)
  set(PYTHON_SOURCE_PACKAGE Python-${MITK_PYTHON_MAJOR_VERSION}.${MITK_PYTHON_MINOR_VERSION}.${MITK_PYTHON_PATCH_VERSION})
  set(proj ${PYTHON_SOURCE_PACKAGE})
  set(proj_DEPENDENCIES )
  set(${proj}_DEPENDS ${proj})

  if(NOT DEFINED Python_DIR)

    # download the source code
    ExternalProject_Add(${proj}
      LIST_SEPARATOR ${sep}
      URL ${MITK_THIRDPARTY_DOWNLOAD_PREFIX_URL}/${PYTHON_SOURCE_PACKAGE}.tgz
      URL_MD5  "2cf641732ac23b18d139be077bd906cd"
      # patch the VS compiler config
      PATCH_COMMAND ${PATCH_COMMAND} -N -p1 -i ${CMAKE_CURRENT_LIST_DIR}/Python-2.7.3.patch
      CONFIGURE_COMMAND ""
      BUILD_COMMAND ""
      INSTALL_COMMAND ""
    )
  else()
    mitkMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")
  endif()

  set(proj Python)
  set(proj_DEPENDENCIES ZLIB ${PYTHON_SOURCE_PACKAGE})
  set(Python_DEPENDS ${proj})

  if(NOT DEFINED Python_DIR)

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
        -DBUILTIN_ZLIB:BOOL=OFF
        -DUSE_SYSTEM_ZLIB:BOOL=ON
        )

    if(CTEST_USE_LAUNCHERS)
      list(APPEND additional_cmake_cache_args
        "-DCMAKE_PROJECT_${proj}_INCLUDE:FILEPATH=${CMAKE_ROOT}/Modules/CTestUseLaunchers.cmake"
      )
    endif()

    # windows has to be installed to a seperate dir, or all py files will end up
    # directly in the ep/lib folder
    set(_python_install_dir )
    if(WIN32)
      set(_python_install_dir -DCMAKE_INSTALL_PREFIX:PATH=${ep_prefix}/lib/python${MITK_PYTHON_MAJOR_VERSION}.${MITK_PYTHON_MINOR_VERSION})
    endif()

    # CMake build environment for python from:
    # https://github.com/davidsansome/python-cmake-buildsystem
    ExternalProject_Add(${proj}
      LIST_SEPARATOR ${sep}
      URL ${MITK_THIRDPARTY_DOWNLOAD_PREFIX_URL}/python-cmake-buildsystem-47845c55.tar.gz
      URL_MD5 "6e49d1ed93a5a0fff7621430c163d2d1"
      # fix to build python on i686 and i386 with the python cmake build system,
      # the x86 path must be set as default
      PATCH_COMMAND ${PATCH_COMMAND} -N -p1 -i ${CMAKE_CURRENT_LIST_DIR}/python-cmake-buildsystem-47845c55.patch
      CMAKE_ARGS
        ${ep_common_args}
      CMAKE_CACHE_ARGS
        ${ep_common_cache_args}
        -DCMAKE_CXX_COMPILER:FILEPATH=${CMAKE_CXX_COMPILER}
        -DCMAKE_C_COMPILER:FILEPATH=${CMAKE_C_COMPILER}
        #-DBUILD_TESTING:BOOL=OFF
        -DBUILD_SHARED:BOOL=ON
        -DBUILD_STATIC:BOOL=OFF
        -DUSE_SYSTEM_LIBRARIES:BOOL=OFF
        ${additional_cmake_cache_args}
        ${_python_install_dir}
        -DZLIB_INCLUDE_DIR:PATH=${ZLIB_INCLUDE_DIR}
        -DZLIB_LIBRARY:FILEPATH=${ZLIB_LIBRARY}
        # Python (and Numpy) do not like different shared library names
        -DCMAKE_DEBUG_POSTFIX:STRING=
      CMAKE_CACHE_DEFAULT_ARGS
        ${ep_common_cache_default_args}
      DEPENDS
        ${proj_DEPENDENCIES}
    )

    # set versions, override
    set(PYTHON_VERSION_MAJOR ${MITK_PYTHON_MAJOR_VERSION})
    set(PYTHON_VERSION_MINOR ${MITK_PYTHON_MINOR_VERSION})

    if(UNIX)
      set(Python_DIR "${ep_prefix}")
      set(PYTHON_EXECUTABLE "${Python_DIR}/bin/python${CMAKE_EXECUTABLE_SUFFIX}")
      set(PYTHON_INCLUDE_DIR "${Python_DIR}/include/python${MITK_PYTHON_MAJOR_VERSION}.${MITK_PYTHON_MINOR_VERSION}")
      set(PYTHON_INCLUDE_DIR2 "${PYTHON_INCLUDE_DIR}")
      set(PYTHON_LIBRARY "${Python_DIR}/lib/${CMAKE_SHARED_LIBRARY_PREFIX}python${MITK_PYTHON_MAJOR_VERSION}.${MITK_PYTHON_MINOR_VERSION}${CMAKE_SHARED_LIBRARY_SUFFIX}")
      set(MITK_PYTHON_SITE_DIR "${Python_DIR}/lib/python${MITK_PYTHON_MAJOR_VERSION}.${MITK_PYTHON_MINOR_VERSION}/site-packages")

      # linux custom compile step, set the environment variables and python home to the right
      # path. Other runtimes can mess up the paths of the installation and the stripped executable
      # used to compile links to the wrong library in /usr/lib
      set(_python_compile_step ${ep_prefix}/tmp/python_compile_step.sh)
      file(WRITE ${_python_compile_step}
"#!/bin/bash
export PYTHONHOME=${Python_DIR}
export LD_LIBRARY_PATH=${Python_DIR}/lib
${PYTHON_EXECUTABLE} -m compileall

")
      # pre compile all *.py files in the runtime after install step
      ExternalProject_Add_Step(${proj} compile_step
        COMMAND sh ${_python_compile_step}
        WORKING_DIRECTORY ${Python_DIR}
        DEPENDEES install
      )

      # use the python executable in the build dir for unix systems. The stripped
      # ones will cause conflicts if system libraries are present during the build/configure process
      # of opencv, since they will try to lookup the sys path first if no lib is directly
      # linked with it s path into the executable
      #ExternalProject_Get_Property(${proj} binary_dir)
      #set(PYTHON_EXECUTABLE "${binary_dir}/bin/python${CMAKE_EXECUTABLE_SUFFIX}")
    else()
      set(Python_DIR "${ep_prefix}/lib/python${MITK_PYTHON_MAJOR_VERSION}.${MITK_PYTHON_MINOR_VERSION}")
      set(PYTHON_EXECUTABLE "${Python_DIR}/bin/python${CMAKE_EXECUTABLE_SUFFIX}")
      set(PYTHON_INCLUDE_DIR "${Python_DIR}/include")
      set(PYTHON_INCLUDE_DIR2 "${PYTHON_INCLUDE_DIR}")
      set(PYTHON_LIBRARY "${Python_DIR}/libs/python${MITK_PYTHON_MAJOR_VERSION}${MITK_PYTHON_MINOR_VERSION}.lib")
      set(MITK_PYTHON_SITE_DIR "${Python_DIR}/Lib/site-packages")

      # pre compile all *.py files in the runtime after install step
      ExternalProject_Add_Step(${proj} compile_step
        COMMAND ${PYTHON_EXECUTABLE} -m compileall
        WORKING_DIRECTORY ${Python_DIR}
        DEPENDEES install
      )
    endif()

    mitkFunctionInstallExternalCMakeProject(${proj})
    install(DIRECTORY ${Python_DIR}
            DESTINATION .
            COMPONENT runtime
            FILES_MATCHING PATTERN "*.pyc"
           )

  else()
    mitkMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")
  endif()
endif()

