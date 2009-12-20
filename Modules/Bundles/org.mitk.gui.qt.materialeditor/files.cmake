SET(SRC_CPP_FILES
  
)

SET(INTERNAL_CPP_FILES
  QmitkMITKSurfaceMaterialEditorView.cpp

)

SET(UI_FILES
  src/internal/QmitkMITKSurfaceMaterialEditorViewControls.ui
)

SET(MOC_H_FILES
  src/internal/QmitkMITKSurfaceMaterialEditorView.h
)

SET(RES_FILES
  resources/QmitkMITKSurfaceMaterialEditorView.qrc
)

SET(CPP_FILES manifest.cpp)

foreach(file ${SRC_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})

