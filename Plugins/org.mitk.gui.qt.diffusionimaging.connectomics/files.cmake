set(SRC_CPP_FILES
  QmitkFreeSurferParcellationHandler.cpp
  QmitkFreeSurferParcellationWidget.cpp
)

set(INTERNAL_CPP_FILES
  QmitkConnectomicsDataView.cpp
  QmitkConnectomicsNetworkOperationsView.cpp
  QmitkConnectomicsStatisticsView.cpp
  QmitkNetworkHistogramCanvas.cpp
  QmitkRandomParcellationView.cpp
  
  Perspectives/QmitkConnectomicsPerspective.cpp
  
  mitkPluginActivator.cpp
)

set(UI_FILES
  src/internal/QmitkConnectomicsDataViewControls.ui
  src/internal/QmitkConnectomicsNetworkOperationsViewControls.ui
  src/internal/QmitkConnectomicsStatisticsViewControls.ui
  src/internal/QmitkRandomParcellationViewControls.ui
  
  src/QmitkFreeSurferParcellationWidgetControls.ui
)

set(MOC_H_FILES
  src/internal/mitkPluginActivator.h
  
  src/internal/Perspectives/QmitkConnectomicsPerspective.h
  
  src/internal/QmitkConnectomicsDataView.h
  src/internal/QmitkConnectomicsNetworkOperationsView.h
  src/internal/QmitkConnectomicsStatisticsView.h
  src/internal/QmitkNetworkHistogramCanvas.h
  src/internal/QmitkRandomParcellationView.h
  
  src/QmitkFreeSurferParcellationHandler.h
  src/QmitkFreeSurferParcellationWidget.h
)

set(CACHED_RESOURCE_FILES
  plugin.xml
  
  resources/QmitkConnectomicsDataViewIcon_48.png
  resources/QmitkConnectomicsNetworkOperationsViewIcon_48.png
  resources/QmitkConnectomicsStatisticsViewIcon_48.png
  resources/QmitkRandomParcellationIcon.png
  resources/parcellation.png
)

set(QRC_FILES
  resources/QmitkConnectomics.qrc
)


set(CPP_FILES )

foreach(file ${SRC_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})
