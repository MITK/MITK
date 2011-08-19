SET(SRC_CPP_FILES
  QmitkDTIAtlasAppApplication.cpp
  QmitkDTIAtlasAppWorkbenchAdvisor.cpp
)

SET(INTERNAL_CPP_FILES
  QmitkDTIAtlasAppApplicationPlugin.cpp
  QmitkDTIAtlasAppIntroPart.cpp
  QmitkDTIAtlasAppPerspective.cpp
  QmitkWelcomePerspective.cpp
)

SET(UI_FILES
 src/internal/QmitkWelcomeScreenViewControls.ui
)

SET(MOC_H_FILES
  src/internal/QmitkDTIAtlasAppIntroPart.h
  src/internal/QmitkDTIAtlasAppApplicationPlugin.h
  src/QmitkDTIAtlasAppApplication.h
  src/internal/QmitkDTIAtlasAppPerspective.h
  src/internal/QmitkWelcomePerspective.h
)

SET(CACHED_RESOURCE_FILES
# list of resource files which can be used by the plug-in
# system without loading the plug-ins shared library,
# for example the icon used in the menu and tabs for the
# plug-in views in the workbench
  plugin.xml
  resources/diffusionimaging.png
)

SET(QRC_FILES
# uncomment the following line if you want to use Qt resources
  resources/welcome/QmitkWelcomeScreenView.qrc
  resources/org_mitk_gui_qt_dtiatlasapp.qrc
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
  SET(_plugin_qhcp_input "${CMAKE_CURRENT_SOURCE_DIR}/documentation/MitkDTIAtlasAppQtHelpCollectionProject.qhcp")
  SET(_plugin_qhcp_output "${PLUGIN_OUTPUT_DIR}/resources/MitkDTIAtlasAppQtHelpCollection_${MBI_WC_REVISION_HASH}.qhc")
  ADD_CUSTOM_COMMAND(OUTPUT ${_plugin_qhcp_output}
                     COMMAND ${QT_COLLECTIONGENERATOR_EXECUTABLE} ${_plugin_qhcp_input} -o ${_plugin_qhcp_output}
                     DEPENDS ${_plugin_qhcp_input}
                     )

  SET(FILE_DEPENDENCIES ${_plugin_qhcp_output})
ENDIF()
