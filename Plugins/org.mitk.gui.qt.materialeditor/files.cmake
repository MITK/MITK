set(SRC_CPP_FILES

)

set(INTERNAL_CPP_FILES
  QmitkMITKSurfaceMaterialEditorView.cpp
  mitkMaterialEditorPluginActivator.cpp
)

set(UI_FILES
  src/internal/QmitkMITKSurfaceMaterialEditorViewControls.ui
)

set(MOC_H_FILES
  src/internal/QmitkMITKSurfaceMaterialEditorView.h
  src/internal/mitkMaterialEditorPluginActivator.h
)

set(CACHED_RESOURCE_FILES
  resources/SurfaceMaterialEditor.png
  plugin.xml
)

set(QRC_FILES
  resources/QmitkMITKSurfaceMaterialEditorView.qrc
)

foreach(file ${SRC_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})