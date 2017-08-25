set(INTERNAL_CPP_FILES
  mitkPluginActivator.cpp
  QmitkSemanticRelationsView.cpp
)

set(UI_FILES
  src/internal/QmitkSemanticRelationsControls.ui
)

set(MOC_H_FILES
  src/internal/mitkPluginActivator.h
  src/internal/QmitkSemanticRelationsView.h
)

set(CACHED_RESOURCE_FILES
  resources/SemanticRelations_48.png
  plugin.xml
)

set(QRC_FILES 
)

foreach(file ${INTERNAL_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})
