set(SRC_CPP_FILES
)

set(INTERNAL_CPP_FILES
  mitkPluginActivator.cpp
  QmitkDiffusionDicomImportView.cpp
  QmitkControlVisualizationPropertiesView.cpp

  Perspectives/QmitkDiffusionImagingAppPerspective.cpp
  Perspectives/QmitkDiffusionDefaultPerspective.cpp
)

set(UI_FILES
  src/internal/QmitkDiffusionDicomImportViewControls.ui
  src/internal/QmitkControlVisualizationPropertiesViewControls.ui
)

set(MOC_H_FILES
  src/internal/mitkPluginActivator.h
  src/internal/QmitkDiffusionDicomImportView.h
  src/internal/QmitkControlVisualizationPropertiesView.h
  
  src/internal/Perspectives/QmitkDiffusionImagingAppPerspective.h
  src/internal/Perspectives/QmitkDiffusionDefaultPerspective.h
)

set(CACHED_RESOURCE_FILES
# list of resource files which can be used by the plug-in
# system without loading the plug-ins shared library,
# for example the icon used in the menu and tabs for the
# plug-in views in the workbench
  plugin.xml

  resources/dwiimport.png
  resources/vizControls.png
  resources/arrow.png
  resources/dwi.png
  resources/qball.png
  resources/tensor.png
  resources/tractogram.png
  resources/refresh.xpm
 )

set(QRC_FILES
# uncomment the following line if you want to use Qt resources
  resources/QmitkDiffusionImaging.qrc
  #resources/QmitkTractbasedSpatialStatisticsView.qrc
)

set(CPP_FILES )

foreach(file ${SRC_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})

