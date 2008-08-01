MACRO(MITK_GENERATE_TOOLS_LIBRARY)

  SET(libraryname ${ARGV0})
  
  SET(reallycreatelibrary TRUE)

  IF (${ARGC} EQUAL 2) # this won't work without the ${}, don't understand the cmake documentation
    SET(reallycreatelibrary FALSE)
  ENDIF (${ARGC} EQUAL 2)

  # if used inside MITK core, we fill variables in files.cmake
  INCLUDE(files.cmake OPTIONAL)



  # GUI-less tools 
  IF (TOOL_FILES)

    # TODO these will also get Qmitk includes! Should not be
    INCLUDE_DIRECTORIES(${MITK_INCLUDE_DIRS})

    FOREACH( TOOL_FILE ${TOOL_FILES}  )

      # construct tool name from file name
      STRING(REGEX REPLACE ".*(^|[/\\])mitk(.+)\\.c(pp|xx)$" "\\2" TOOL_NAME ${TOOL_FILE})
      MESSAGE(STATUS "Adding tool ${TOOL_NAME}")

      # source file names for this tool
      SET( TOOL_CPPS ${TOOL_FILE} ${TOOL_CPPS} )

      # configure a file that contains an itkObjectFactory for this tool
      SET( FACTORY_NAME ${TOOL_NAME}Factory )
      SET( FACTORY_DESCRIPTION "Generated factory for ${TOOL_NAME}" )
      CONFIGURE_FILE( ${MITK_DIR}/ToolExtensionITKFactory.cpp.in 
                      ${PROJECT_BINARY_DIR}/mitk${FACTORY_NAME}.h
                      @ONLY )
      INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})

      # remember factory name for a central register call
      SET( FACTORY_INCLUDE_LINES "#include \"${CMAKE_CURRENT_BINARY_DIR}/mitk${FACTORY_NAME}.h\"
${FACTORY_INCLUDE_LINES}" )
      SET( FACTORY_REGISTER_CALLS "${FACTORY_REGISTER_CALLS} 
           itk::ObjectFactoryBase::RegisterFactory( mitk::${FACTORY_NAME}::New() );" )
    ENDFOREACH( TOOL_FILE ${TOOL_FILES}  )

  ENDIF (TOOL_FILES)



  # part for Qt3 widgets
  IF (TOOL_QT3GUI_FILES)

    # give them Qmitk headers
    INCLUDE_DIRECTORIES(${QMITK_INCLUDE_DIRS})
    
    FOREACH( TOOL_GUI_FILE ${TOOL_QT3GUI_FILES})  

      # construct tool name from file name
      STRING(REGEX REPLACE "^Qmitk(.+)GUI\\.c(pp|xx)$" "\\1" TOOL_NAME ${TOOL_GUI_FILE})
      STRING(REGEX REPLACE "\\.c(pp|xx)$" ".h" TOOL_GUI_HEADER ${TOOL_GUI_FILE})
      MESSAGE(STATUS "Adding tool GUI ${TOOL_NAME}")

      # source file names for this tool
      SET( TOOL_GUI_CPPS ${TOOL_GUI_FILE} ${TOOL_GUI_CPPS} )
      SET( TOOL_GUI_MOC_H ${TOOL_GUI_HEADER} ${TOOL_GUI_MOC_H} )

      # configure a file that contains an itkObjectFactory for this tool
      STRING(REGEX REPLACE "(.+)\\.c(pp|xx)$" "\\1Factory" FACTORY_NAME ${TOOL_GUI_FILE})
      SET( FACTORY_DESCRIPTION "Generated factory for Qmitk${TOOL_NAME}GUI" )
      CONFIGURE_FILE( ${MITK_DIR}/ToolGUIExtensionITKFactory.cpp.in 
                      ${PROJECT_BINARY_DIR}/${FACTORY_NAME}.h
                      @ONLY )
      INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})

      # remember factory name for a central register call
      SET( FACTORY_INCLUDE_LINES "#include \"${PROJECT_BINARY_DIR}/${FACTORY_NAME}.h\"
${FACTORY_INCLUDE_LINES}" )
      SET( FACTORY_REGISTER_CALLS "${FACTORY_REGISTER_CALLS}
           itk::ObjectFactoryBase::RegisterFactory( ${FACTORY_NAME}::New() );" )
    ENDFOREACH( TOOL_GUI_FILE ${TOOL_QT3GUI_FILES})  

    QT_WRAP_CPP(${libraryname} TOOL_GUI_CPPS ${TOOL_GUI_MOC_H})
  ENDIF (TOOL_QT3GUI_FILES)

  

  # care for additional files (should be used only with MITK external extensions)
  IF (TOOL_ADDITIONAL_CPPS)
    SET( TOOL_CPPS ${TOOL_ADDITIONAL_CPPS} ${TOOL_CPPS} )
  ENDIF (TOOL_ADDITIONAL_CPPS)

  IF (TOOL_ADDITIONAL_MOC_H)
    QT_WRAP_CPP(${libraryname} TOOL_GUI_CPPS ${TOOL_ADDITIONAL_MOC_H})
  ENDIF (TOOL_ADDITIONAL_MOC_H)

  # in any case (GUI or non-GUI), create a shared library
  IF (TOOL_FILES OR TOOL_QT3GUI_FILES)
    IF (libraryname AND reallycreatelibrary)
      # configure one file with the itkLoad method
      CONFIGURE_FILE( ${MITK_DIR}/ToolExtensionITKFactoryLoader.cpp.in
                      ${PROJECT_BINARY_DIR}/mitkToolExtensionITKFactoryLoader.cpp
                      @ONLY )
      # also mark this file for compilation
      SET( TOOL_CPPS ${PROJECT_BINARY_DIR}/mitkToolExtensionITKFactoryLoader.cpp ${TOOL_CPPS} )
      MESSAGE(STATUS "Creating itkLoad for external library " ${libraryname})
    
    MESSAGE(STATUS "Adding library " ${libraryname})

    # a library to wrap up everything
    LINK_DIRECTORIES(${MITK_LINK_DIRECTORIES})
    ADD_LIBRARY(${libraryname} SHARED ${TOOL_CPPS} ${TOOL_GUI_CPPS})
    
    TARGET_LINK_LIBRARIES(${libraryname} ${QMITK_LIBRARIES})
    ENDIF (libraryname AND reallycreatelibrary)
  ENDIF (TOOL_FILES OR TOOL_QT3GUI_FILES)



ENDMACRO(MITK_GENERATE_TOOLS_LIBRARY)

