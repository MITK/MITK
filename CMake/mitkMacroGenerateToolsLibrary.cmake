macro(MITK_GENERATE_TOOLS_LIBRARY)

  set(libraryname ${ARGV0})

  set(reallycreatelibrary TRUE)

  if(${ARGC} EQUAL 2) # this won't work without the ${}, don't understand the cmake documentation
    set(reallycreatelibrary FALSE)
  endif(${ARGC} EQUAL 2)

  # if used inside MITK core, we fill variables in files.cmake
  include(tools.cmake OPTIONAL)



  # GUI-less tools
  if(TOOL_FILES)

    # TODO these will also get Qmitk includes! Should not be
    include_directories(${MITK_INCLUDE_DIRS})

    foreach( TOOL_FILE ${TOOL_FILES}  )

      # construct tool name from file name
      string(REGEX REPLACE ".*(^|[/\\])mitk(.+)\\.c(pp|xx)$" "\\2" TOOL_NAME ${TOOL_FILE})
      message(STATUS "Adding segmentation tool: ${TOOL_NAME}")

      # source file names for this tool
      set( TOOL_CPPS ${TOOL_FILE} ${TOOL_CPPS} )

      # remember factory name for a central register call
    endforeach( TOOL_FILE ${TOOL_FILES}  )

  endif(TOOL_FILES)



  # part for Qt widgets
  if(TOOL_QT4GUI_FILES)

    # give them Qmitk headers
    include_directories(${QMITK_INCLUDE_DIRS})

    foreach( TOOL_GUI_FILE ${TOOL_QT4GUI_FILES})

      # construct tool name from file name
      string(REGEX REPLACE "^Qmitk(.+)GUI\\.c(pp|xx)$" "\\1" TOOL_NAME ${TOOL_GUI_FILE})
      string(REGEX REPLACE "\\.c(pp|xx)$" ".h" TOOL_GUI_HEADER ${TOOL_GUI_FILE})
      message(STATUS "Adding GUI for segmentation tool: ${TOOL_NAME}")

      # source file names for this tool
      set( TOOL_GUI_CPPS ${TOOL_GUI_FILE} ${TOOL_GUI_CPPS} )
      set( TOOL_GUI_MOC_H ${TOOL_GUI_HEADER} ${TOOL_GUI_MOC_H} )
    endforeach( TOOL_GUI_FILE ${TOOL_QT4GUI_FILES})

    qt_wrap_cpp(${libraryname} TOOL_GUI_CPPS ${TOOL_GUI_MOC_H})
  endif(TOOL_QT4GUI_FILES)



  # care for additional files (should be used only with MITK external extensions)
  if(TOOL_ADDITIONAL_CPPS)
    set( TOOL_CPPS ${TOOL_ADDITIONAL_CPPS} ${TOOL_CPPS} )
  endif(TOOL_ADDITIONAL_CPPS)

  if(TOOL_ADDITIONAL_MOC_H)
    qt_wrap_cpp(${libraryname} TOOL_GUI_CPPS ${TOOL_ADDITIONAL_MOC_H})
  endif(TOOL_ADDITIONAL_MOC_H)

  # in any case (GUI or non-GUI), create a shared library
  if(TOOL_FILES OR TOOL_QT4GUI_FILES)
    if(libraryname AND reallycreatelibrary)
      # configure one file with the itkLoad method
      configure_file( ${MITK_DIR}/ToolExtensionITKFactoryLoader.cpp.in
                      ${PROJECT_BINARY_DIR}/mitkToolExtensionITKFactoryLoader.cpp
                      @ONLY )
      # also mark this file for compilation
      set( TOOL_CPPS ${PROJECT_BINARY_DIR}/mitkToolExtensionITKFactoryLoader.cpp ${TOOL_CPPS} )
      message(STATUS "Creating itkLoad for external library " ${libraryname})

    message(STATUS "Adding library " ${libraryname})

    # a library to wrap up everything
    # configure a file that contains an itkObjectFactory for this tool
    include_directories(${CMAKE_CURRENT_SOURCE_DIR})

    link_directories(${MITK_LINK_DIRECTORIES})
    add_library(${libraryname} SHARED ${TOOL_CPPS} ${TOOL_GUI_CPPS})

    target_link_libraries(${libraryname} ${QMITK_LIBRARIES})
    endif(libraryname AND reallycreatelibrary)
  endif(TOOL_FILES OR TOOL_QT4GUI_FILES)


endmacro(MITK_GENERATE_TOOLS_LIBRARY)

