#------------------------------------------------------------------
# ZLIB
#------------------------------------------------------------------
if(MITK_USE_ZLIB)
  if(NOT DEFINED ZLIB_DIR)
    set(proj ZLIB)
    set(${proj}_DEPENDENCIES )
    set(ZLIB_DEPENDS ${proj})

    # Using the ZLIB from CTK:
    # https://github.com/commontk/zlib
    ExternalProject_Add(${proj}
      URL ${MITK_THIRDPARTY_DOWNLOAD_PREFIX_URL}/zlib-66a75305.tar.gz
      URL_MD5 "4c3f572b487ae7947fd88ec363533bc5"
      SOURCE_DIR ${CMAKE_BINARY_DIR}/${proj}-src
      BINARY_DIR ${proj}-build
      PREFIX ${proj}-cmake
      INSTALL_DIR ${proj}-install
      CMAKE_ARGS
      ${ep_common_args}
      CMAKE_CACHE_ARGS
      -DBUILD_SHARED_LIBS:BOOL=OFF
      -DZLIB_MANGLE_PREFIX:STRING=mitk_zlib_
      -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
      DEPENDS
      ${ZLIB_DEPENDENCIES}
      )
    set(ZLIB_DIR ${CMAKE_BINARY_DIR}/${proj}-install)
    set(ZLIB_INCLUDE_DIR ${ZLIB_DIR}/include)
    if(WIN32)
      set(ZLIB_LIBRARY ${ZLIB_DIR}/lib/zlib.lib)
    else()
      set(ZLIB_LIBRARY ${ZLIB_DIR}/lib/libzlib.a)
    endif()
  else()
    mitkMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")
  endif()
endif()

