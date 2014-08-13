set(SRC_CPP_FILES
  QmitkDiffusionImagingAppApplication.cpp
  QmitkDiffusionImagingAppWorkbenchAdvisor.cpp
)

set(INTERNAL_CPP_FILES
  QmitkDiffusionApplicationPlugin.cpp
  QmitkDiffusionImagingAppIntroPart.cpp
  Perspectives/QmitkWelcomePerspective.cpp
)

set(UI_FILES
  src/internal/QmitkWelcomeScreenViewControls.ui
)

set(MOC_H_FILES
  src/internal/QmitkDiffusionImagingAppIntroPart.h
  src/internal/QmitkDiffusionApplicationPlugin.h
  src/QmitkDiffusionImagingAppApplication.h
  src/internal/Perspectives/QmitkWelcomePerspective.h
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

