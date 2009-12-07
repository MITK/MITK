SET(SRC_CPP_FILES
  QmitkExtApplication.cpp
  QmitkExtAppWorkbenchAdvisor.cpp
)

SET(INTERNAL_CPP_FILES
  QmitkExtApplicationPlugin.cpp
  QmitkExtDefaultPerspective.cpp
)

SET(MOC_H_FILES
 
)

SET(RESOURCE_FILES
# list of resource files which can be used by the plug-in
# system without loading the plug-ins shared library,
# for example the icon used in the menu and tabs for the
# plug-in views in the workbench
  resources/icon_research.xpm
)

SET(CPP_FILES manifest.cpp)

foreach(file ${SRC_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})

#----------- Qt Help Collection Project -------------#
FIND_PACKAGE(Subversion)
IF(Subversion_FOUND AND MITK_USE_SUBVERSION)
  Subversion_WC_INFO(${PROJECT_SOURCE_DIR} SVN_INFO)
  SET(MITK_SVN_REVISION "_${SVN_INFO_WC_REVISION}")
ELSE()
  SET(MITK_SVN_REVISION "")
ENDIF()

IF (OPENCHERRY_USE_QT_HELP)
  SET(_plugin_qhcp_input "${CMAKE_CURRENT_SOURCE_DIR}/documentation/MitkExtQtHelpCollectionProject.qhcp")
  SET(_plugin_qhcp_output "${PLUGIN_OUTPUT_DIR}/resources/MitkExtQtHelpCollection${MITK_SVN_REVISION}.qhc")
  ADD_CUSTOM_COMMAND(OUTPUT ${_plugin_qhcp_output}
                     COMMAND ${QT_COLLECTIONGENERATOR_EXECUTABLE} ${_plugin_qhcp_input} -o ${_plugin_qhcp_output}
                     DEPENDS ${_plugin_qhcp_input}
                     )
                     
  SET(FILE_DEPENDENCIES ${_plugin_qhcp_output})
ENDIF()