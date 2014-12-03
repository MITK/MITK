set(SRC_CPP_FILES
)

set(INTERNAL_CPP_FILES
  QmitkAnimationItem.cpp
  QmitkAnimationItemDelegate.cpp
  QmitkAnimationWidget.cpp
  QmitkFFmpegWriter.cpp
  QmitkMovieMakerView.cpp
  QmitkOrbitAnimationItem.cpp
  QmitkOrbitAnimationWidget.cpp
  QmitkSliceAnimationItem.cpp
  QmitkSliceAnimationWidget.cpp
  QmitkTimeSliceAnimationItem.cpp
  QmitkTimeSliceAnimationWidget.cpp
  mitkMovieMakerPluginActivator.cpp
  QmitkScreenshotMaker.cpp
)

set(UI_FILES
  src/internal/QmitkMovieMakerView.ui
  src/internal/QmitkOrbitAnimationWidget.ui
  src/internal/QmitkSliceAnimationWidget.ui
  src/internal/QmitkTimeSliceAnimationWidget.ui
  src/internal/QmitkScreenshotMakerControls.ui
)

set(MOC_H_FILES
  src/internal/mitkMovieMakerPluginActivator.h
  src/internal/QmitkAnimationItemDelegate.h
  src/internal/QmitkAnimationWidget.h
  src/internal/QmitkFFmpegWriter.h
  src/internal/QmitkMovieMakerView.h
  src/internal/QmitkOrbitAnimationWidget.h
  src/internal/QmitkSliceAnimationWidget.h
  src/internal/QmitkTimeSliceAnimationWidget.h
  src/internal/QmitkScreenshotMaker.h
)

set(CACHED_RESOURCE_FILES
  resources/camera-video.png
  resources/screenshot_maker.png
  plugin.xml
)

set(QRC_FILES
 resources/QmitkMovieMaker.qrc
)

foreach(file ${SRC_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})

