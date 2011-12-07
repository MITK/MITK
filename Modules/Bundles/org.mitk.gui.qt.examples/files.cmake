SET(SRC_CPP_FILES
  
)

SET(INTERNAL_CPP_FILES
  mitkPluginActivator.cpp
  colourimageprocessing/QmitkColourImageProcessingView.cpp
  colourimageprocessing/mitkColourImageProcessor.cpp
  isosurface/QmitkIsoSurface.cpp
  regiongrowing/QmitkRegionGrowingView.cpp
  simpleexample/QmitkSimpleExampleView.cpp
  simplemeasurement/QmitkSimpleMeasurement.cpp
  viewinitialization/QmitkViewInitializationView.cpp
  volumetry/QmitkVolumetryView.cpp
)

SET(UI_FILES
  src/internal/colourimageprocessing/QmitkColourImageProcessingViewControls.ui
  src/internal/isosurface/QmitkIsoSurfaceControls.ui
  src/internal/regiongrowing/QmitkRegionGrowingViewControls.ui
  src/internal/simpleexample/QmitkSimpleExampleViewControls.ui
  src/internal/simplemeasurement/QmitkSimpleMeasurementControls.ui
  src/internal/viewinitialization/QmitkViewInitializationViewControls.ui
  src/internal/volumetry/QmitkVolumetryViewControls.ui
)

SET(MOC_H_FILES
  src/internal/mitkPluginActivator.h
  src/internal/colourimageprocessing/QmitkColourImageProcessingView.h
  src/internal/isosurface/QmitkIsoSurface.h
  src/internal/regiongrowing/QmitkRegionGrowingView.h
  src/internal/simpleexample/QmitkSimpleExampleView.h
  src/internal/simplemeasurement/QmitkSimpleMeasurement.h
  src/internal/viewinitialization/QmitkViewInitializationView.h
  src/internal/volumetry/QmitkVolumetryView.h
)

SET(CACHED_RESOURCE_FILES
  plugin.xml
  resources/ColorImageProcessing.png
  resources/isoSurface.xpm
  resources/regiongrowing.xpm
  resources/SimpleExample.png
  resources/SimpleMeasurement.png
  resources/viewInitializationIcon.xpm
  resources/volumetryIcon.xpm
)

SET(QRC_FILES
  resources/QmitkColourImageProcessingView.qrc
  resources/isosurface.qrc
  resources/regiongrowing.qrc
  resources/QmitkVolumetryView.qrc
)

SET(CPP_FILES )

foreach(file ${SRC_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})

