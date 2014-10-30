set(SRC_CPP_FILES
)

set(INTERNAL_CPP_FILES
  org_mitk_gui_qt_moviemaker2_Activator.cpp
  QmitkMovieMaker2View.cpp
)

set(UI_FILES
  src/internal/QmitkMovieMaker2ViewControls.ui
)

set(MOC_H_FILES
  src/internal/org_mitk_gui_qt_moviemaker2_Activator.h
  src/internal/QmitkMovieMaker2View.h
)

set(CACHED_RESOURCE_FILES
  resources/camera-video.png
  plugin.xml
)

set(QRC_FILES
  resources/org_mitk_gui_qt_moviemaker2.qrc
)

set(CPP_FILES
)

foreach(file ${SRC_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/${file})
endforeach()

foreach(file ${INTERNAL_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach()
