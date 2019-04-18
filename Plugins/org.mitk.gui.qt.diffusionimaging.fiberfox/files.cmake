set(SRC_CPP_FILES
  QmitkTensorModelParametersWidget.cpp
  QmitkZeppelinModelParametersWidget.cpp
  QmitkStickModelParametersWidget.cpp
  QmitkDotModelParametersWidget.cpp
  QmitkBallModelParametersWidget.cpp
  QmitkAstrosticksModelParametersWidget.cpp
  QmitkPrototypeSignalParametersWidget.cpp
)

set(INTERNAL_CPP_FILES
  QmitkFiberfoxView.cpp
  QmitkFieldmapGeneratorView.cpp
  QmitkFiberGenerationView.cpp
  mitkPluginActivator.cpp
  
  Perspectives/QmitkDIAppSyntheticDataGenerationPerspective.cpp
)

set(UI_FILES  
  src/internal/QmitkFiberfoxViewControls.ui
  src/internal/QmitkFieldmapGeneratorViewControls.ui
  src/internal/QmitkFiberGenerationViewControls.ui
  
  src/QmitkTensorModelParametersWidgetControls.ui
  src/QmitkZeppelinModelParametersWidgetControls.ui
  src/QmitkStickModelParametersWidgetControls.ui
  src/QmitkDotModelParametersWidgetControls.ui
  src/QmitkBallModelParametersWidgetControls.ui
  src/QmitkAstrosticksModelParametersWidgetControls.ui
  src/QmitkPrototypeSignalParametersWidgetControls.ui
)

set(MOC_H_FILES
  src/internal/mitkPluginActivator.h
  src/internal/QmitkFiberfoxView.h
  src/internal/QmitkFieldmapGeneratorView.h
  src/internal/QmitkFiberGenerationView.h
  
  src/QmitkTensorModelParametersWidget.h
  src/QmitkZeppelinModelParametersWidget.h
  src/QmitkStickModelParametersWidget.h
  src/QmitkDotModelParametersWidget.h
  src/QmitkBallModelParametersWidget.h
  src/QmitkAstrosticksModelParametersWidget.h
  src/QmitkPrototypeSignalParametersWidget.h
  
  src/internal/Perspectives/QmitkDIAppSyntheticDataGenerationPerspective.h
)

set(CACHED_RESOURCE_FILES
  plugin.xml
  
  resources/phantom.png
  resources/syntheticdata.png
  resources/fieldmap.png
  resources/models.png
  resources/download.png
  resources/right.png
  resources/stop.png
  resources/upload.png
)

set(QRC_FILES
    resources/QmitkFiberfox.qrc
)


set(CPP_FILES )

foreach(file ${SRC_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})
