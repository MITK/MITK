SET(SRC_CPP_FILES

)

SET(INTERNAL_CPP_FILES
  QmitkIGTTrackingLabView.cpp
  QmitkIGTTutorialView.cpp
  mitkPluginActivator.cpp
)

SET(UI_FILES
  src/internal/QmitkIGTTrackingLabViewControls.ui
  src/internal/QmitkIGTTutorialViewControls.ui
)

SET(MOC_H_FILES
  src/internal/QmitkIGTTrackingLabView.h
  src/internal/QmitkIGTTutorialView.h
  src/internal/mitkPluginActivator.h
)

SET(QRC_FILES
  resources/QmitkIGTTrackingLabView.qrc
  #resources/QmitkIGTTutorialView.qrc
)

SET(CACHED_RESOURCE_FILES
  resources/IgtTrackingLab.png
  resources/start_rec.png
  resources/stop_rec.png
  resources/IGTTutorial.png
  plugin.xml
)

SET(CPP_FILES

)

foreach(file ${SRC_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})
