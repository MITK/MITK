set(SRC_CPP_FILES
  QmitkTbssRoiAnalysisWidget.cpp
)

set(INTERNAL_CPP_FILES
  QmitkTractbasedSpatialStatisticsView.cpp
  QmitkTbssTableModel.cpp
  QmitkTbssMetaTableModel.cpp
  QmitkTbssSkeletonizationView.cpp
  
  mitkPluginActivator.cpp
)

set(UI_FILES
  src/internal/QmitkTractbasedSpatialStatisticsViewControls.ui
  src/internal/QmitkTbssSkeletonizationViewControls.ui
)

set(MOC_H_FILES
  src/internal/mitkPluginActivator.h
  
  src/internal/QmitkTractbasedSpatialStatisticsView.h
  src/internal/QmitkTbssSkeletonizationView.h
  src/QmitkTbssRoiAnalysisWidget.h
)

set(CACHED_RESOURCE_FILES
  plugin.xml
  
  resources/tbss.png
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
