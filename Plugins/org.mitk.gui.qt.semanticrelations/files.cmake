set(INTERNAL_CPP_FILES
  mitkPluginActivator.cpp
  QmitkDataNodeAddToSemanticRelationsAction.cpp
  QmitkDataNodeRemoveFromSemanticRelationsAction.cpp
  QmitkDataNodeUnlinkFromLesionAction.cpp
  QmitkDataNodeSetControlPointAction.cpp
  QmitkDataNodeSetInformationTypeAction.cpp
  QmitkDataSetOpenInAction.cpp
  QmitkFocusOnLesionAction.cpp
  QmitkLabelSetJumpToAction.cpp
  QmitkLesionInfoWidget.cpp
  QmitkSemanticRelationsContextMenu.cpp
  QmitkSemanticRelationsNodeSelectionDialog.cpp
  QmitkSemanticRelationsStatisticsView.cpp
  QmitkSemanticRelationsView.cpp
)

set(UI_FILES
  src/internal/QmitkLesionInfoWidgetControls.ui
  src/internal/QmitkSemanticRelationsControls.ui
  src/internal/QmitkSemanticRelationsStatisticsControls.ui
)

set(MOC_H_FILES
  src/internal/mitkPluginActivator.h
  src/internal/QmitkDataNodeAddToSemanticRelationsAction.h
  src/internal/QmitkDataNodeRemoveFromSemanticRelationsAction.h
  src/internal/QmitkDataNodeUnlinkFromLesionAction.h
  src/internal/QmitkDataNodeSetControlPointAction.h
  src/internal/QmitkDataNodeSetInformationTypeAction.h
  src/internal/QmitkDataSetOpenInAction.h
  src/internal/QmitkFocusOnLesionAction.h
  src/internal/QmitkLabelSetJumpToAction.h
  src/internal/QmitkLesionInfoWidget.h
  src/internal/QmitkSemanticRelationsContextMenu.h
  src/internal/QmitkSemanticRelationsNodeSelectionDialog.h
  src/internal/QmitkSemanticRelationsStatisticsView.h
  src/internal/QmitkSemanticRelationsView.h
)

set(CACHED_RESOURCE_FILES
  resources/SemanticRelations_48.png
  resources/SemanticRelationsStatistics_48.png
  plugin.xml
)

set(QRC_FILES 
)

foreach(file ${INTERNAL_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})
