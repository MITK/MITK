SET(SRC_CPP_FILES
  QmitkDiffusionImagingAppApplication.cpp
  QmitkDiffusionImagingAppWorkbenchAdvisor.cpp
)

SET(INTERNAL_CPP_FILES
  mitkPluginActivator.cpp
  QmitkDiffusionImagingAppIntroPart.cpp
  QmitkDiffusionImagingAppPerspective.cpp
  QmitkWelcomePerspective.cpp
  QmitkDIAppConnectomicsPerspective.cpp
  QmitkDIAppDicomImportPerspective.cpp
  QmitkDIAppFiberTractographyPerspective.cpp
  QmitkDIAppIVIMPerspective.cpp
  QmitkDIAppPreprocessingReconstructionPerspective.cpp
  QmitkDIAppQuantificationPerspective.cpp
  QmitkDIAppScreenshotsMoviesPerspective.cpp
  QmitkDIAppTBSSPerspective.cpp
  QmitkDIAppVolumeVisualizationPerspective.cpp
)

SET(UI_FILES
  src/internal/QmitkWelcomeScreenViewControls.ui
)

SET(MOC_H_FILES
  src/internal/QmitkDiffusionImagingAppIntroPart.h
  src/internal/mitkPluginActivator.h
  src/QmitkDiffusionImagingAppApplication.h
  src/internal/QmitkDiffusionImagingAppPerspective.h
  src/internal/QmitkWelcomePerspective.h
  src/internal/QmitkDIAppConnectomicsPerspective.h
  src/internal/QmitkDIAppDicomImportPerspective.h
  src/internal/QmitkDIAppFiberTractographyPerspective.h
  src/internal/QmitkDIAppIVIMPerspective.h
  src/internal/QmitkDIAppPreprocessingReconstructionPerspective.h
  src/internal/QmitkDIAppQuantificationPerspective.h
  src/internal/QmitkDIAppScreenshotsMoviesPerspective.h
  src/internal/QmitkDIAppTBSSPerspective.h
  src/internal/QmitkDIAppVolumeVisualizationPerspective.h
)

SET(CACHED_RESOURCE_FILES
# list of resource files which can be used by the plug-in
# system without loading the plug-ins shared library,
# for example the icon used in the menu and tabs for the
# plug-in views in the workbench
  plugin.xml
  resources/icon_dicom.xpm
  resources/diffusionimaging.png
  resources/preprocessing.png
  resources/Measurement_48.png
  resources/volvis.png
  resources/perspectives/diffusionimaging.png
  resources/perspectives/icon_home.png
  resources/perspectives/connectomics.png
  resources/perspectives/dicomimport.png
  resources/perspectives/tractography.png
  resources/perspectives/ivim.png
  resources/perspectives/preprocessingreconstruction.png
  resources/perspectives/quantification.png
  resources/perspectives/screenshotsmovies.png
  resources/perspectives/tbss.png
  resources/perspectives/volumevizualization.png
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
