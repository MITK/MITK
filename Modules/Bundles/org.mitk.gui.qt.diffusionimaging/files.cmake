SET(SRC_CPP_FILES
  
)

SET(INTERNAL_CPP_FILES
  QmitkQBallReconstructionView.cpp
  QmitkDiffusionDicomImportView.cpp
  QmitkDiffusionQuantificationView.cpp
  QmitkTensorReconstructionView.cpp
  #QmitkDiffusionImagingPerspective.cpp
  QmitkControlVisualizationPropertiesView.cpp
)

SET(UI_FILES
  src/internal/QmitkQBallReconstructionViewControls.ui
  src/internal/QmitkDiffusionDicomImportViewControls.ui
  src/internal/QmitkDiffusionQuantificationViewControls.ui
  src/internal/QmitkTensorReconstructionViewControls.ui
  src/internal/QmitkControlVisualizationPropertiesViewControls.ui
)

SET(MOC_H_FILES
  src/internal/QmitkQBallReconstructionView.h
  src/internal/QmitkDiffusionDicomImportView.h
  src/internal/QmitkDiffusionQuantificationView.h
  src/internal/QmitkTensorReconstructionView.h
  src/internal/QmitkControlVisualizationPropertiesView.h
)

SET(RESOURCE_FILES
# list of resource files which can be used by the plug-in
# system without loading the plug-ins shared library,
# for example the icon used in the menu and tabs for the
# plug-in views in the workbench
  resources/dwi.png
  resources/tensor.png
  resources/qball.png
  resources/dwiimport.png
  resources/quantification.png
  resources/reconodf.png
  resources/recontensor.png
  resources/stats.png
  resources/texIntOFFIcon.png
  resources/texIntONIcon.png
  resources/vizControls.png
  resources/Refresh_48.png
  resources/QBallData24.png
)

SET(RES_FILES
# uncomment the following line if you want to use Qt resources
  resources/QmitkDiffusionImaging.qrc
)

SET(CPP_FILES manifest.cpp)

foreach(file ${SRC_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})

