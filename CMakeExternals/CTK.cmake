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

  SET(additional_cmake_args)
  IF(MITK_USE_Python)
    LIST(APPEND additional_cmake_args
         -DCTK_LIB_Scripting/Python/Widgets:BOOL=ON
        )
  ENDIF()


  IF(NOT DEFINED CTK_DIR)
    ExternalProject_Add(${proj}
      GIT_REPOSITORY git://github.com/commontk/CTK.git
      BINARY_DIR ${proj}-build
      INSTALL_COMMAND ""
      CMAKE_GENERATOR ${gen}
      CMAKE_ARGS
        ${ep_common_args}
        ${additional_cmake_args}
        -DDESIRED_QT_VERSION:STRING=4
        -DQT_QMAKE_EXECUTABLE:FILEPATH=${QT_QMAKE_EXECUTABLE}
        -DCTK_LIB_DICOM/Widgets:BOOL=ON
      DEPENDS ${proj_DEPENDENCIES}
     )
  SET(CTK_DIR ${CMAKE_CURRENT_BINARY_DIR}/${proj}-build)
  
  ELSE()

    mitkMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")
    
  ENDIF()
  
ENDIF()
