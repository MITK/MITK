#-----------------------------------------------------------------------------
# CTK
#-----------------------------------------------------------------------------

if(MITK_USE_CTK)

  # Sanity checks
  if(DEFINED CTK_DIR AND NOT EXISTS ${CTK_DIR})
    message(FATAL_ERROR "CTK_DIR variable is defined but corresponds to non-existing directory")
  endif()

  set(proj CTK)
  mitk_query_custom_ep_vars()

  set(proj_DEPENDENCIES DCMTK ${${proj}_CUSTOM_DEPENDENCIES})
  set(CTK_DEPENDS ${proj})

  if(NOT DEFINED CTK_DIR)

    set(ctk_optional_cache_args )

    if(CTEST_USE_LAUNCHERS)
      list(APPEND ctk_optional_cache_args
        "-DCMAKE_PROJECT_${proj}_INCLUDE:FILEPATH=${CMAKE_ROOT}/Modules/CTestUseLaunchers.cmake"
      )
    endif()

    FOREACH(type RUNTIME ARCHIVE LIBRARY)
      IF(DEFINED CTK_PLUGIN_${type}_OUTPUT_DIRECTORY)
        LIST(APPEND mitk_optional_cache_args -DCTK_PLUGIN_${type}_OUTPUT_DIRECTORY:PATH=${CTK_PLUGIN_${type}_OUTPUT_DIRECTORY})
      ENDIF()
    ENDFOREACH()

    ExternalProject_Add(${proj}
      LIST_SEPARATOR ${sep}
      GIT_REPOSITORY https://github.com/MITK/CTK.git
      GIT_TAG 37aff99226ed936b930b1ef07df046877771ea03 # branch: qt-6
      UPDATE_COMMAND ""
      INSTALL_COMMAND ""
      CMAKE_GENERATOR ${gen}
      CMAKE_GENERATOR_PLATFORM ${gen_platform}
      CMAKE_ARGS
        ${ep_common_args}
        ${ctk_optional_cache_args}
        "-DQt6_DIR:PATH=${Qt6_DIR}"
        # The CTK PluginFramework cannot cope with
        # a non-empty CMAKE_DEBUG_POSTFIX for the plugin
        # libraries yet.
        -DCMAKE_DEBUG_POSTFIX:STRING=
        -DCTK_QT_VERSION:STRING=6
        "-DGIT_EXECUTABLE:FILEPATH=${GIT_EXECUTABLE}"
        -DCTK_BUILD_QTDESIGNER_PLUGINS:BOOL=OFF
        -DCTK_LIB_PluginFramework:BOOL=ON
        -DCTK_LIB_DICOM/Widgets:BOOL=ON
        -DCTK_LIB_XNAT/Core:BOOL=ON
        -DCTK_PLUGIN_org.commontk.eventadmin:BOOL=ON
        -DCTK_PLUGIN_org.commontk.configadmin:BOOL=ON
        -DCTK_USE_SYSTEM_DCMTK:BOOL=ON
        "-DDCMTK_ROOT:PATH=${ep_prefix}"
        ${${proj}_CUSTOM_CMAKE_ARGS}
      CMAKE_CACHE_ARGS
        ${ep_common_cache_args}
        ${${proj}_CUSTOM_CMAKE_CACHE_ARGS}
      CMAKE_CACHE_DEFAULT_ARGS
        ${ep_common_cache_default_args}
        ${${proj}_CUSTOM_CMAKE_CACHE_DEFAULT_ARGS}
      DEPENDS ${proj_DEPENDENCIES}
     )

    ExternalProject_Get_Property(${proj} binary_dir)
    set(CTK_DIR ${binary_dir})

  else()

    mitkMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")

  endif()

endif()
