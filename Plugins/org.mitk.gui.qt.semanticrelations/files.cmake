set(INTERNAL_CPP_FILES
  mitkPluginActivator.cpp
  QmitkDataNodeAddToSemanticRelationsAction.cpp
  QmitkDataNodeInformationTypeAction.cpp
  QmitkDataNodeRemoveFromSemanticRelationsAction.cpp
  QmitkDataNodeUnlinkFromLesionAction.cpp
  QmitkDataNodeOpenInAction.cpp
  QmitkDataNodeSetControlPointAction.cpp
  QmitkLesionInfoWidget.cpp
  QmitkSemanticRelationsContextMenu.cpp
  QmitkSemanticRelationsNodeSelectionDialog.cpp
  QmitkSemanticRelationsView.cpp
)

set(UI_FILES
  src/internal/QmitkLesionInfoWidgetControls.ui
  src/internal/QmitkSemanticRelationsControls.ui
)

set(MOC_H_FILES
  src/internal/mitkPluginActivator.h
  src/internal/QmitkDataNodeAddToSemanticRelationsAction.h
  src/internal/QmitkDataNodeInformationTypeAction.h
  src/internal/QmitkDataNodeRemoveFromSemanticRelationsAction.h
  src/internal/QmitkDataNodeUnlinkFromLesionAction.h
  src/internal/QmitkDataNodeOpenInAction.h
  src/internal/QmitkDataNodeSetControlPointAction.h
  src/internal/QmitkLesionInfoWidget.h
  src/internal/QmitkSemanticRelationsContextMenu.h
  src/internal/QmitkSemanticRelationsNodeSelectionDialog.h
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
