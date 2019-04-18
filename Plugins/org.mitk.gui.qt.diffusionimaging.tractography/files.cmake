set(INTERNAL_CPP_FILES
  QmitkGibbsTrackingView.cpp
  QmitkStreamlineTrackingView.cpp

  Perspectives/QmitkGibbsTractographyPerspective.cpp
  Perspectives/QmitkStreamlineTractographyPerspective.cpp

  mitkPluginActivator.cpp
)

set(UI_FILES
  src/internal/QmitkGibbsTrackingViewControls.ui
  src/internal/QmitkStreamlineTrackingViewControls.ui
)

set(MOC_H_FILES
  src/internal/mitkPluginActivator.h

  src/internal/QmitkGibbsTrackingView.h
  src/internal/QmitkStreamlineTrackingView.h

  src/internal/Perspectives/QmitkGibbsTractographyPerspective.h
  src/internal/Perspectives/QmitkStreamlineTractographyPerspective.h
)

set(CACHED_RESOURCE_FILES
  plugin.xml

  resources/tract.png
  resources/tractogram.png
  resources/ml_tractography.png
  resources/download.png
  resources/right.png
  resources/stop.png
  resources/upload.png
)

set(QRC_FILES
  resources/QmitkTractography.qrc
)


set(CPP_FILES )

foreach(file ${SRC_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})
