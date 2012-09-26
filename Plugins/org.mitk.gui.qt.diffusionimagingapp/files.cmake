set(SRC_CPP_FILES
  QmitkDiffusionImagingAppApplication.cpp
  QmitkDiffusionImagingAppWorkbenchAdvisor.cpp
)

set(INTERNAL_CPP_FILES
  QmitkDiffusionApplicationPlugin.cpp
  QmitkDiffusionImagingAppIntroPart.cpp
  Perspectives/QmitkDiffusionImagingAppPerspective.cpp
  Perspectives/QmitkWelcomePerspective.cpp
  Perspectives/QmitkDIAppConnectomicsPerspective.cpp
  Perspectives/QmitkDIAppDicomImportPerspective.cpp
  Perspectives/QmitkDIAppFiberTractographyPerspective.cpp
  Perspectives/QmitkDIAppIVIMPerspective.cpp
  Perspectives/QmitkDIAppPreprocessingReconstructionPerspective.cpp
  Perspectives/QmitkDIAppQuantificationPerspective.cpp
  Perspectives/QmitkDIAppTBSSPerspective.cpp
  Perspectives/QmitkDIAppUtilityPerspective.cpp
  Perspectives/QmitkDIAppImageProcessingPerspective.cpp
  Perspectives/QmitkDIAppSyntheticDataGenerationPerspective.cpp
  Perspectives/QmitkDIAppRegistrationPerspective.cpp
  Perspectives/QmitkDIAppVisualizationPerspective.cpp
)

set(UI_FILES
  src/internal/QmitkWelcomeScreenViewControls.ui
)

set(MOC_H_FILES
  src/internal/QmitkDiffusionImagingAppIntroPart.h
  src/internal/QmitkDiffusionApplicationPlugin.h
  src/QmitkDiffusionImagingAppApplication.h
  src/internal/Perspectives/QmitkDiffusionImagingAppPerspective.h
  src/internal/Perspectives/QmitkWelcomePerspective.h
  src/internal/Perspectives/QmitkDIAppConnectomicsPerspective.h
  src/internal/Perspectives/QmitkDIAppDicomImportPerspective.h
  src/internal/Perspectives/QmitkDIAppFiberTractographyPerspective.h
  src/internal/Perspectives/QmitkDIAppIVIMPerspective.h
  src/internal/Perspectives/QmitkDIAppPreprocessingReconstructionPerspective.h
  src/internal/Perspectives/QmitkDIAppQuantificationPerspective.h
  src/internal/Perspectives/QmitkDIAppTBSSPerspective.h
  src/internal/Perspectives/QmitkDIAppUtilityPerspective.h
  src/internal/Perspectives/QmitkDIAppImageProcessingPerspective.h
  src/internal/Perspectives/QmitkDIAppSyntheticDataGenerationPerspective.h
  src/internal/Perspectives/QmitkDIAppRegistrationPerspective.h
  src/internal/Perspectives/QmitkDIAppVisualizationPerspective.h
)

set(CACHED_RESOURCE_FILES
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
  resources/perspectives/tbss.png
  resources/perspectives/utilities.png
  resources/perspectives/imageProcessing.png
  resources/perspectives/registration.png
  resources/perspectives/phantomData2.png
  resources/perspectives/eye.png
  resources/perspectives/registration.xpm
  resources/perspectives/chart.png
  resources/perspectives/preprocessing.png
)

set(QRC_FILES
# uncomment the following line if you want to use Qt resources
  resources/welcome/QmitkWelcomeScreenView.qrc
  resources/org_mitk_gui_qt_diffusionimagingapp.qrc
)

# set(CPP_FILES)

foreach(file ${SRC_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})

