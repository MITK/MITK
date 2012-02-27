SET(SRC_CPP_FILES
  
)

SET(INTERNAL_CPP_FILES
  QmitkMITKSurfaceMaterialEditorView.cpp
  mitkMaterialEditorPluginActivator.cpp
)

SET(UI_FILES
  src/internal/QmitkMITKSurfaceMaterialEditorViewControls.ui
)

SET(MOC_H_FILES
  src/internal/QmitkMITKSurfaceMaterialEditorView.h
  src/internal/mitkMaterialEditorPluginActivator.h
)

SET(CACHED_RESOURCE_FILES
  resources/SurfaceMaterialEditor.png
  plugin.xml
)

SET(QRC_FILES
  resources/QmitkMITKSurfaceMaterialEditorView.qrc
)

foreach(file ${SRC_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})