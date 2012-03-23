set(SRC_CPP_FILES
  QmitkDTIAtlasAppApplication.cpp
  QmitkDTIAtlasAppWorkbenchAdvisor.cpp
)

set(INTERNAL_CPP_FILES
  QmitkDTIAtlasAppApplicationPlugin.cpp
  QmitkDTIAtlasAppIntroPart.cpp
  QmitkDTIAtlasAppPerspective.cpp
  QmitkWelcomePerspective.cpp
)

set(UI_FILES
 src/internal/QmitkWelcomeScreenViewControls.ui
)

set(MOC_H_FILES
  src/internal/QmitkDTIAtlasAppIntroPart.h
  src/internal/QmitkDTIAtlasAppApplicationPlugin.h
  src/QmitkDTIAtlasAppApplication.h
  src/internal/QmitkDTIAtlasAppPerspective.h
  src/internal/QmitkWelcomePerspective.h
)

set(CACHED_RESOURCE_FILES
# list of resource files which can be used by the plug-in
# system without loading the plug-ins shared library,
# for example the icon used in the menu and tabs for the
# plug-in views in the workbench
  plugin.xml
  resources/diffusionimaging.png
)

set(QRC_FILES
# uncomment the following line if you want to use Qt resources
  resources/welcome/QmitkWelcomeScreenView.qrc
  resources/org_mitk_gui_qt_dtiatlasapp.qrc
)

# set(CPP_FILES)

foreach(file ${SRC_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})


#----------- Qt Help Collection Project -------------#

if(BLUEBERRY_USE_QT_HELP)
  set(_plugin_qhcp_input "${CMAKE_CURRENT_SOURCE_DIR}/documentation/MitkDTIAtlasAppQtHelpCollectionProject.qhcp")
  set(_plugin_qhcp_output "${PLUGIN_OUTPUT_DIR}/resources/MitkDTIAtlasAppQtHelpCollection_${MBI_WC_REVISION_HASH}.qhc")
  add_custom_command(OUTPUT ${_plugin_qhcp_output}
                     COMMAND ${QT_COLLECTIONGENERATOR_EXECUTABLE} ${_plugin_qhcp_input} -o ${_plugin_qhcp_output}
                     DEPENDS ${_plugin_qhcp_input}
                     )

  set(FILE_DEPENDENCIES ${_plugin_qhcp_output})
endif()
