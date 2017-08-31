#-----------------------------------------------------------------------------
# DCMTK
#-----------------------------------------------------------------------------

if(MITK_USE_DCMTK)

  # Sanity checks
  if(DEFINED DCMTK_DIR AND NOT EXISTS ${DCMTK_DIR})
    message(FATAL_ERROR "DCMTK_DIR variable is defined but corresponds to non-existing directory")
  endif()

  set(proj DCMTK)
  set(proj_DEPENDENCIES )
  set(DCMTK_DEPENDS ${proj})

  if(NOT DEFINED DCMTK_DIR)
    if(DCMTK_DICOM_ROOT_ID)
      set(DCMTK_CXX_FLAGS "${DCMTK_CXX_FLAGS} -DSITE_UID_ROOT=\\\"${DCMTK_DICOM_ROOT_ID}\\\"")
      set(DCMTK_C_FLAGS "${DCMTK_CXX_FLAGS} -DSITE_UID_ROOT=\\\"${DCMTK_DICOM_ROOT_ID}\\\"")
    endif()

    set(additional_args )
    if(CTEST_USE_LAUNCHERS)
      list(APPEND additional_args
        "-DCMAKE_PROJECT_${proj}_INCLUDE:FILEPATH=${CMAKE_ROOT}/Modules/CTestUseLaunchers.cmake"
      )
    endif()

    ExternalProject_Add(${proj}
      LIST_SEPARATOR ${sep}
      # specific version post-3.6.2 but before an API change which breaks DCMQI
      URL ${MITK_THIRDPARTY_DOWNLOAD_PREFIX_URL}/dcmtk_065a4b1f.tar.gz
      URL_MD5 9362231aba67812deed65b35308de8bd
      CMAKE_GENERATOR ${gen}
      CMAKE_ARGS
         ${ep_common_args}
         ${additional_args}
         "-DCMAKE_CXX_FLAGS:STRING=${CMAKE_CXX_FLAGS} ${DCMTK_CXX_FLAGS}"
         "-DCMAKE_C_FLAGS:STRING=${CMAKE_C_FLAGS} ${DCMTK_C_FLAGS}"
         -DDCMTK_ENABLE_CXX11:BOOL=ON
         -DDCMTK_ENABLE_STL:BOOL=ON
         -DDCMTK_WITH_DOXYGEN:BOOL=OFF
         -DDCMTK_WITH_ZLIB:BOOL=OFF # see bug #9894
         -DDCMTK_WITH_OPENSSL:BOOL=OFF # see bug #9894
         -DDCMTK_WITH_PNG:BOOL=OFF # see bug #9894
         -DDCMTK_WITH_TIFF:BOOL=OFF  # see bug #9894
         -DDCMTK_WITH_XML:BOOL=OFF  # see bug #9894
         -DDCMTK_WITH_ICONV:BOOL=OFF  # see bug #9894
      CMAKE_CACHE_ARGS
        ${ep_common_cache_args}
      CMAKE_CACHE_DEFAULT_ARGS
        ${ep_common_cache_default_args}
      DEPENDS ${proj_DEPENDENCIES}
      )
    set(DCMTK_DIR ${ep_prefix})
    mitkFunctionInstallExternalCMakeProject(${proj})

  else()

    mitkMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")

  endif()
endif()
