set(SRC_CPP_FILES
  QmitkMlbstTrainingDataWidget.cpp
)

set(INTERNAL_CPP_FILES
  QmitkGibbsTrackingView.cpp
  QmitkStreamlineTrackingView.cpp
  QmitkMLBTView.cpp

  Perspectives/QmitkGibbsTractographyPerspective.cpp
  Perspectives/QmitkStreamlineTractographyPerspective.cpp

  mitkPluginActivator.cpp
)

set(UI_FILES
  src/internal/QmitkGibbsTrackingViewControls.ui
  src/internal/QmitkStreamlineTrackingViewControls.ui
  src/internal/QmitkMLBTViewControls.ui

  src/QmitkMlbstTrainingDataWidgetControls.ui
)

set(MOC_H_FILES
  src/internal/mitkPluginActivator.h

  src/internal/QmitkGibbsTrackingView.h
  src/internal/QmitkStreamlineTrackingView.h
  src/internal/QmitkMLBTView.h

  src/QmitkMlbstTrainingDataWidget.h

  src/internal/Perspectives/QmitkGibbsTractographyPerspective.h
  src/internal/Perspectives/QmitkStreamlineTractographyPerspective.h
)

set(CACHED_RESOURCE_FILES
  plugin.xml

  resources/tract.png
  resources/tractogram.png
  resources/ml_tractography.png
)

set(QRC_FILES

)


set(CPP_FILES )

foreach(file ${SRC_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})
