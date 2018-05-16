set(SRC_CPP_FILES
  QmitkODFDetailsWidget.cpp
  QmitkODFRenderWidget.cpp
  QmitkResidualAnalysisWidget.cpp
  QmitkResidualViewWidget.cpp
)

set(INTERNAL_CPP_FILES
  mitkPluginActivator.cpp

  Perspectives/QmitkReconstructionPerspective.cpp
  Perspectives/QmitkQuantificationPerspective.cpp

  QmitkQBallReconstructionView.cpp
  QmitkDiffusionQuantificationView.cpp
  QmitkTensorReconstructionView.cpp
  QmitkODFDetailsView.cpp
)

set(UI_FILES
  src/internal/QmitkQBallReconstructionViewControls.ui
  src/internal/QmitkDiffusionQuantificationViewControls.ui
  src/internal/QmitkTensorReconstructionViewControls.ui
  src/internal/QmitkODFDetailsViewControls.ui
)

set(MOC_H_FILES
  src/internal/mitkPluginActivator.h

  src/internal/Perspectives/QmitkReconstructionPerspective.h
  src/internal/Perspectives/QmitkQuantificationPerspective.h

  src/internal/QmitkQBallReconstructionView.h
  src/internal/QmitkDiffusionQuantificationView.h
  src/internal/QmitkTensorReconstructionView.h
  src/internal/QmitkODFDetailsView.h

  src/QmitkODFRenderWidget.h
  src/QmitkODFDetailsWidget.h
  src/QmitkResidualAnalysisWidget.h
  src/QmitkResidualViewWidget.h
)

set(CACHED_RESOURCE_FILES
  plugin.xml

  resources/quantification.png
  resources/OdfDetails.png
  resources/tensor.png
  resources/odf.png
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
