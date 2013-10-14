#-----------------------------------------------------------------------------
# CTK
#-----------------------------------------------------------------------------

if(MITK_USE_CTK)

  # Sanity checks
  if(DEFINED CTK_DIR AND NOT EXISTS ${CTK_DIR})
    message(FATAL_ERROR "CTK_DIR variable is defined but corresponds to non-existing directory")
  endif()

  set(proj CTK)
  set(proj_DEPENDENCIES )
  set(CTK_DEPENDS ${proj})

  if(NOT DEFINED CTK_DIR)

    set(revision_tag f18114d2)
    #IF(${proj}_REVISION_TAG)
    #  SET(revision_tag ${${proj}_REVISION_TAG})
    #ENDIF()

    set(ctk_optional_cache_args )
    if(MITK_USE_Python)
      list(APPEND ctk_optional_cache_args
           -DCTK_LIB_Scripting/Python/Widgets:BOOL=ON
           -DCTK_ENABLE_Python_Wrapping:BOOL=ON
           -DCTK_APP_ctkSimplePythonShell:BOOL=ON
           -DPYTHON_EXECUTABLE:FILEPATH=${PYTHON_EXECUTABLE}
           -DPYTHON_INCLUDE_DIR:PATH=${PYTHON_INCLUDE_DIR}
           -DPYTHON_INCLUDE_DIR2:PATH=${PYTHON_INCLUDE_DIR2}
           -DPYTHON_LIBRARY:FILEPATH=${PYTHON_LIBRARY}
      )
    else()
      list(APPEND ctk_optional_cache_args
           -DCTK_LIB_Scripting/Python/Widgets:BOOL=OFF
           -DCTK_ENABLE_Python_Wrapping:BOOL=OFF
           -DCTK_APP_ctkSimplePythonShell:BOOL=OFF
      )
    endif()

    if(MITK_USE_DCMTK)
      list(APPEND ctk_optional_cache_args
           -DDCMTK_DIR:PATH=${DCMTK_DIR}
          )
      list(APPEND proj_DEPENDENCIES DCMTK)
    else()
      list(APPEND ctk_optional_cache_args
           -DDCMTK_URL:STRING=${MITK_THIRDPARTY_DOWNLOAD_PREFIX_URL}/CTK_DCMTK_085525e6.tar.gz
          )
    endif()

    FOREACH(type RUNTIME ARCHIVE LIBRARY)
      IF(DEFINED CTK_PLUGIN_${type}_OUTPUT_DIRECTORY)
        LIST(APPEND mitk_optional_cache_args -DCTK_PLUGIN_${type}_OUTPUT_DIRECTORY:PATH=${CTK_PLUGIN_${type}_OUTPUT_DIRECTORY})
      ENDIF()
    ENDFOREACH()

    ExternalProject_Add(${proj}
      SOURCE_DIR ${CMAKE_BINARY_DIR}/${proj}-src
      BINARY_DIR ${proj}-build
      PREFIX ${proj}-cmake
      URL ${MITK_THIRDPARTY_DOWNLOAD_PREFIX_URL}/CTK_${revision_tag}.tar.gz
      URL_MD5 03694d11d206a94284af6801aa9d8b5c
      UPDATE_COMMAND ""
      INSTALL_COMMAND ""
      CMAKE_GENERATOR ${gen}
      CMAKE_ARGS
        ${ep_common_args}
        ${ctk_optional_cache_args}
        -DDESIRED_QT_VERSION:STRING=${DESIRED_QT_VERSION}
        -DQT_QMAKE_EXECUTABLE:FILEPATH=${QT_QMAKE_EXECUTABLE}
        -DGit_EXECUTABLE:FILEPATH=${GIT_EXECUTABLE}
        -DGIT_EXECUTABLE:FILEPATH=${GIT_EXECUTABLE}
        -DCTK_LIB_CommandLineModules/Backend/LocalProcess:BOOL=ON
        -DCTK_LIB_CommandLineModules/Frontend/QtGui:BOOL=ON
        -DCTK_LIB_PluginFramework:BOOL=ON
        -DCTK_LIB_DICOM/Widgets:BOOL=ON
        -DCTK_PLUGIN_org.commontk.eventadmin:BOOL=ON
        -DCTK_PLUGIN_org.commontk.configadmin:BOOL=ON
        -DCTK_USE_GIT_PROTOCOL:BOOL=OFF
        -DDCMTK_URL:STRING=${MITK_THIRDPARTY_DOWNLOAD_PREFIX_URL}/CTK_DCMTK_085525e6.tar.gz
      DEPENDS ${proj_DEPENDENCIES}
     )
  set(CTK_DIR ${CMAKE_CURRENT_BINARY_DIR}/${proj}-build)

  else()

    mitkMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")

  endif()

endif()
