set(SRC_CPP_FILES

)

set(INTERNAL_CPP_FILES
  QmitkImageStatisticsView.cpp
  QmitkImageStatisticsCalculationThread.cpp
  mitkPluginActivator.cpp
)

set(UI_FILES
  src/internal/QmitkImageStatisticsViewControls.ui
)

set(MOC_H_FILES
  src/internal/QmitkImageStatisticsView.h
  src/internal/QmitkImageStatisticsCalculationThread.h
  src/internal/mitkPluginActivator.h

)

set(CACHED_RESOURCE_FILES
  resources/ImageStatistic_24.png
  plugin.xml
)

set(QRC_FILES
  resources/QmitkImageStatisticsView.qrc
)


set(CPP_FILES )

foreach(file ${SRC_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})
