SET(SRC_CPP_FILES
  QmitkDiffusionImagingAppApplication.cpp
  QmitkDiffusionImagingAppWorkbenchAdvisor.cpp
)

SET(INTERNAL_CPP_FILES
  QmitkDiffusionImagingAppApplicationPlugin.cpp
  QmitkDiffusionImagingAppIntroPart.cpp
  QmitkDiffusionImagingAppPerspective.cpp
  QmitkWelcomePerspective.cpp
)

SET(UI_FILES
 src/internal/QmitkWelcomeScreenViewControls.ui
)

SET(MOC_H_FILES
  src/internal/QmitkDiffusionImagingAppIntroPart.h
  src/internal/QmitkDiffusionImagingAppApplicationPlugin.h
  src/QmitkDiffusionImagingAppApplication.h
  src/internal/QmitkDiffusionImagingAppPerspective.h
  src/internal/QmitkWelcomePerspective.h
)

SET(CACHED_RESOURCE_FILES
# list of resource files which can be used by the plug-in
# system without loading the plug-ins shared library,
# for example the icon used in the menu and tabs for the
# plug-in views in the workbench
  plugin.xml
  resources/mintLogo.png
)

SET(QRC_FILES
# uncomment the following line if you want to use Qt resources
  resources/welcome/QmitkWelcomeScreenView.qrc
  resources/org_mitk_gui_qt_diffusionimagingapp.qrc
)

# SET(CPP_FILES)

foreach(file ${SRC_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})


#----------- Qt Help Collection Project -------------#

IF (BLUEBERRY_USE_QT_HELP)
  SET(_plugin_qhcp_input "${CMAKE_CURRENT_SOURCE_DIR}/documentation/MitkDiffusionImagingAppQtHelpCollectionProject.qhcp")
  SET(_plugin_qhcp_output "${PLUGIN_OUTPUT_DIR}/resources/MitkDiffusionImagingAppQtHelpCollection_${MBI_WC_REVISION_HASH}.qhc")
  ADD_CUSTOM_COMMAND(OUTPUT ${_plugin_qhcp_output}
                     COMMAND ${QT_COLLECTIONGENERATOR_EXECUTABLE} ${_plugin_qhcp_input} -o ${_plugin_qhcp_output}
                     DEPENDS ${_plugin_qhcp_input}
                     )

  SET(FILE_DEPENDENCIES ${_plugin_qhcp_output})
ENDIF()
