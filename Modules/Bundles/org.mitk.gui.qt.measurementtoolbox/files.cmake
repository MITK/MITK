SET(SRC_CPP_FILES
  
)

SET(INTERNAL_CPP_FILES
  QmitkMeasurementView.cpp
  QmitkPlanarFiguresTableModel.cpp
  QmitkImageStatisticsView.cpp
  mitkPluginActivator.cpp
)

SET(UI_FILES
  src/internal/QmitkImageStatisticsViewControls.ui
)

SET(MOC_H_FILES
  src/internal/QmitkMeasurementView.h
  src/internal/QmitkPlanarFiguresTableModel.h
  src/internal/QmitkImageStatisticsView.h
  src/internal/mitkPluginActivator.h
  
)

SET(CACHED_RESOURCE_FILES
  resources/angle.png
  resources/arrow.png
  resources/circle.png
  resources/four-point-angle.png
  resources/ImageStatistic_24.png
  resources/ImageStatistic_48.png
  resources/ImageStatistic_64.png
  resources/lena.xpm
  resources/line.png
  resources/measurement.png
  resources/path.png
  resources/polygon.png
  resources/rectangle.png
  resources/stats.png
  resources/text.png
  plugin.xml
)

SET(QRC_FILES
  resources/measurement.qrc
  resources/QmitkImageStatisticsView.qrc
)


SET(CPP_FILES )

foreach(file ${SRC_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})
