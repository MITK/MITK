#-----------------------------------------------------------------------------
# CTK
#-----------------------------------------------------------------------------

IF(MITK_USE_CTK)

  # Sanity checks
  IF(DEFINED CTK_DIR AND NOT EXISTS ${CTK_DIR})
    MESSAGE(FATAL_ERROR "CTK_DIR variable is defined but corresponds to non-existing directory")
  ENDIF()

  SET(proj CTK)
  SET(proj_DEPENDENCIES )
  SET(CTK_DEPENDS ${proj})

  IF(NOT DEFINED CTK_DIR)
    
    SET(revision_tag 31d144ecab0c7a8bdc5d8e2ec8b33972ce8e490d)
    IF(${proj}_REVISION_TAG)
      SET(revision_tag ${${proj}_REVISION_TAG})
    ENDIF()
    
    SET(ctk_optional_cache_args )
    IF(MITK_USE_Python)
      LIST(APPEND ctk_optional_cache_args
           -DCTK_LIB_Scripting/Python/Widgets:BOOL=ON
      )
    ENDIF()
    FOREACH(type RUNTIME ARCHIVE LIBRARY)
      IF(DEFINED CTK_PLUGIN_${type}_OUTPUT_DIRECTORY)
        LIST(APPEND mitk_optional_cache_args -DCTK_PLUGIN_${type}_OUTPUT_DIRECTORY:PATH=${CTK_PLUGIN_${type}_OUTPUT_DIRECTORY})
      ENDIF()
    ENDFOREACH()
  
    ExternalProject_Add(${proj}
      GIT_REPOSITORY http://github.com/Miluba154/CTK.git
      GIT_TAG ${revision_tag}
      SOURCE_DIR ${proj}-src
      BINARY_DIR ${proj}-build
      UPDATE_COMMAND ""
      INSTALL_COMMAND ""
      CMAKE_GENERATOR ${gen}
      CMAKE_ARGS
        ${ep_common_args}
        ${ctk_optional_cache_args}
        -DDESIRED_QT_VERSION:STRING=4
        -DQT_QMAKE_EXECUTABLE:FILEPATH=${QT_QMAKE_EXECUTABLE}
        -DGit_EXECUTABLE:FILEPATH=${GIT_EXECUTABLE}
        -DGIT_EXECUTABLE:FILEPATH=${GIT_EXECUTABLE}
        -DCTK_LIB_PluginFramework:BOOL=ON
        -DCTK_LIB_DICOM/Widgets:BOOL=ON
        -DCTK_PLUGIN_org.commontk.eventadmin:BOOL=ON
        -DCTK_PLUGIN_org.commontk.configadmin:BOOL=ON
        -DCTK_USE_GIT_PROTOCOL:BOOL=OFF
      DEPENDS ${proj_DEPENDENCIES}
     )
  SET(CTK_DIR ${CMAKE_CURRENT_BINARY_DIR}/${proj}-build)
  
  ELSE()

    mitkMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")
    
  ENDIF()
  
ENDIF()
