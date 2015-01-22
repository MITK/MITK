#-----------------------------------------------------------------------------
# Poco
#-----------------------------------------------------------------------------

if(MITK_USE_Poco)

  # Sanity checks
  if(DEFINED Poco_DIR AND NOT EXISTS ${Poco_DIR})
    message(FATAL_ERROR "Poco_DIR variable is defined but corresponds to non-existing directory")
  endif()

  set(proj Poco)
  set(proj_DEPENDENCIES )
  set(${proj}_DEPENDS ${proj})

  if(NOT DEFINED ${proj}_DIR)

    ExternalProject_Add(${proj}
      LIST_SEPARATOR ${sep}
      URL ${MITK_THIRDPARTY_DOWNLOAD_PREFIX_URL}/poco-1.6.0-all.tar.gz
      URL_MD5 4fed893d4ca57db98b0e10d82545232a
      PATCH_COMMAND ${PATCH_COMMAND} -N -p1 -i ${CMAKE_CURRENT_LIST_DIR}/Poco-1.6.0.patch
      CMAKE_GENERATOR ${gen}
      CMAKE_ARGS
        ${ep_common_args}
        -DENABLE_XML:BOOL=ON
        -DENABLE_JSON:BOOL=ON
        -DENABLE_MONGODB:BOOL=OFF
        -DENABLE_PDF:BOOL=OFF
        -DENABLE_UTIL:BOOL=ON
        -DENABLE_NET:BOOL=OFF
        -DENABLE_NETSSL:BOOL=OFF
        -DENABLE_NETSSL_WIN:BOOL=OFF
        -DENABLE_CRYPTO:BOOL=OFF
        -DENABLE_DATA:BOOL=OFF
        -DENABLE_DATA_SQLITE:BOOL=OFF
        -DENABLE_DATA_MYSQL:BOOL=OFF
        -DENABLE_DATA_ODBC:BOOL=OFF
        -DENABLE_SEVENZIP:BOOL=OFF
        -DENABLE_ZIP:BOOL=ON
        -DENABLE_APACHECONNECTOR:BOOL=OFF
        -DENABLE_CPPPARSER:BOOL=OFF
        -DENABLE_POCODOC:BOOL=OFF
        -DENABLE_PAGECOMPILER:BOOL=OFF
        -DENABLE_PAGECOMPILER_FILE2PAGE:BOOL=OFF
      DEPENDS ${proj_DEPENDENCIES}
     )

    set(${proj}_DIR ${ep_prefix})
    mitkFunctionInstallExternalCMakeProject(${proj})

  else()

    mitkMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")

  endif()

endif()
