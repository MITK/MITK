SET(SRC_CPP_FILES
  
)

SET(INTERNAL_CPP_FILES
  QmitkQBallReconstructionView.cpp
  QmitkPreprocessingView.cpp
  QmitkDiffusionDicomImportView.cpp
  QmitkDiffusionQuantificationView.cpp
  QmitkTensorReconstructionView.cpp
  QmitkDiffusionImagingPublicPerspective.cpp
  QmitkControlVisualizationPropertiesView.cpp  
)

SET(UI_FILES
  src/internal/QmitkQBallReconstructionViewControls.ui
  src/internal/QmitkPreprocessingViewControls.ui
  src/internal/QmitkDiffusionDicomImportViewControls.ui
  src/internal/QmitkDiffusionQuantificationViewControls.ui
  src/internal/QmitkTensorReconstructionViewControls.ui
  src/internal/QmitkControlVisualizationPropertiesViewControls.ui  
)

SET(MOC_H_FILES
  src/internal/QmitkQBallReconstructionView.h
  src/internal/QmitkPreprocessingView.h
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
  resources/preprocessing.png
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
  resources/glyphsoff_C.png
  resources/glyphsoff_S.png
  resources/glyphsoff_T.png
  resources/glyphson_C.png
  resources/glyphson_S.png
  resources/glyphson_T.png
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

