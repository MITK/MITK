SET(SRC_CPP_FILES
  
)

SET(INTERNAL_CPP_FILES
  QmitkImageStatisticsView.cpp
  mitkImageStatisticsPluginActivator.cpp
)

SET(UI_FILES
  src/internal/QmitkImageStatisticsViewControls.ui
)

SET(MOC_H_FILES
  src/internal/mitkImageStatisticsPluginActivator.h
  src/internal/QmitkImageStatisticsView.h
)

SET(CACHED_RESOURCE_FILES
  resources/stats.png
  resources/ImageStatistic_24.png
  plugin.xml
)

SET(QRC_FILES
# uncomment the following line if you want to use Qt resources
  resources/QmitkImageStatisticsView.qrc
)


foreach(file ${SRC_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})