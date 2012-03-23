set(SRC_CPP_FILES

)

set(INTERNAL_CPP_FILES
  QmitkIGTTrackingLabView.cpp
  QmitkIGTTutorialView.cpp
  mitkPluginActivator.cpp
)

set(UI_FILES
  src/internal/QmitkIGTTrackingLabViewControls.ui
  src/internal/QmitkIGTTutorialViewControls.ui
)

set(MOC_H_FILES
  src/internal/QmitkIGTTrackingLabView.h
  src/internal/QmitkIGTTutorialView.h
  src/internal/mitkPluginActivator.h
)

set(QRC_FILES
  resources/QmitkIGTTrackingLabView.qrc
  #resources/QmitkIGTTutorialView.qrc
)

set(CACHED_RESOURCE_FILES
  resources/IgtTrackingLab.png
  resources/start_rec.png
  resources/stop_rec.png
  resources/IGTTutorial.png
  plugin.xml
)

set(CPP_FILES

)

foreach(file ${SRC_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})
