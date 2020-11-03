set(SRC_CPP_FILES

)

set(INTERNAL_CPP_FILES
  QmitkRadiomicsStatisticView.cpp
  QmitkRadiomicsTransformationView.cpp
  QmitkRadiomicsArithmetricView.cpp
  QmitkRadiomicsMaskProcessingView.cpp
  
  
  mitkBasicImageProcessingActivator.cpp
  QmitkGIFConfigurationPanel.cpp
)

set(UI_FILES
  src/internal/QmitkRadiomicsStatisticViewControls.ui
  src/internal/QmitkRadiomicsTransformationViewControls.ui
  src/internal/QmitkRadiomicsArithmetricViewControls.ui
  src/internal/QmitkRadiomicsMaskProcessingViewControls.ui
)

set(MOC_H_FILES
  src/internal/QmitkRadiomicsStatisticView.h
  src/internal/QmitkRadiomicsTransformationView.h
  src/internal/QmitkRadiomicsArithmetricView.h
  src/internal/QmitkRadiomicsMaskProcessingView.h
  
  
  src/internal/mitkBasicImageProcessingActivator.h
  src/internal/QmitkGIFConfigurationPanel.h
)

set(CACHED_RESOURCE_FILES
  resources/lena.xpm
  resources/Icon_Transformation.png
  resources/bar-chart-radiomics.svg
  resources/transformation-radiomics.svg
  resources/arithmetric-radiomics.svg
  resources/maskcleaning-radiomics.svg
  plugin.xml
)

set(QRC_FILES
  resources/QmitkRadiomicsStatisticView.qrc
)

foreach(file ${SRC_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})

