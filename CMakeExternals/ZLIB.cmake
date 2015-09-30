#------------------------------------------------------------------
# ZLIB
#------------------------------------------------------------------
if(MITK_USE_ZLIB)

  set(proj ZLIB)
  set(${proj}_DEPENDENCIES )
  set(ZLIB_DEPENDS ${proj})

  if(NOT DEFINED ZLIB_DIR)

    set(additional_cmake_args )
    if(CTEST_USE_LAUNCHERS)
      list(APPEND additional_cmake_args
        "-DCMAKE_PROJECT_${proj}_INCLUDE:FILEPATH=${CMAKE_ROOT}/Modules/CTestUseLaunchers.cmake"
      )
    endif()

    # Using the ZLIB from CTK:
    # https://github.com/commontk/zlib
    ExternalProject_Add(${proj}
      LIST_SEPARATOR ${sep}
      URL ${MITK_THIRDPARTY_DOWNLOAD_PREFIX_URL}/zlib-66a75305.tar.gz
      URL_MD5 "4c3f572b487ae7947fd88ec363533bc5"
      CMAKE_ARGS
        ${ep_common_args}
        ${additional_cmake_args}
      CMAKE_CACHE_ARGS
        ${ep_common_cache_args}
        -DBUILD_SHARED_LIBS:BOOL=OFF
        -DZLIB_MANGLE_PREFIX:STRING=mitk_zlib_
        -DZLIB_INSTALL_INCLUDE_DIR:STRING=include/mitk_zlib
      CMAKE_CACHE_DEFAULT_ARGS
        ${ep_common_cache_default_args}
      DEPENDS ${ZLIB_DEPENDENCIES}
      )
    set(ZLIB_DIR ${ep_prefix})
    set(ZLIB_INCLUDE_DIR ${ZLIB_DIR}/include/mitk_zlib)

    install(DIRECTORY ${ZLIB_INCLUDE_DIR}
            DESTINATION include
            COMPONENT dev)

    find_library(ZLIB_LIBRARY_RELEASE NAMES zlib
                 PATHS ${ZLIB_DIR}
                 PATH_SUFFIXES lib lib/Release
                 NO_DEFAULT_PATH)
    find_library(ZLIB_LIBRARY_DEBUG NAMES zlibd
                 PATHS ${ZLIB_DIR}
                 PATH_SUFFIXES lib lib/Debug
                 NO_DEFAULT_PATH)

    set(ZLIB_LIBRARY )
    if(ZLIB_LIBRARY_RELEASE)
      list(APPEND ZLIB_LIBRARY optimized ${ZLIB_LIBRARY_RELEASE})
      install(FILES ${ZLIB_LIBRARY_RELEASE}
              DESTINATION lib
              CONFIGURATIONS Release
              COMPONENT dev)
    endif()
    if(ZLIB_LIBRARY_DEBUG)
      list(APPEND ZLIB_LIBRARY debug ${ZLIB_LIBRARY_DEBUG})
      install(FILES ${ZLIB_LIBRARY_DEBUG}
              DESTINATION lib
              CONFIGURATIONS Debug
              COMPONENT dev)
    endif()

  else()
    mitkMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")
  endif()
endif()

