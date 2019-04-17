set(SRC_CPP_FILES
  QmitkAbstractDataNodeAction.cpp
  QmitkCloseProjectAction.cpp
  QmitkDataNodeColorAction.cpp
  QmitkDataNodeColorMapAction.cpp
  QmitkDataNodeComponentAction.cpp
  QmitkDataNodeContextMenu.cpp
  QmitkDataNodeGlobalReinitAction.cpp
  QmitkDataNodeHideAllAction.cpp
  QmitkDataNodeOpacityAction.cpp
  QmitkDataNodeOpenInAction.cpp
  QmitkDataNodeReinitAction.cpp
  QmitkDataNodeRemoveAction.cpp
  QmitkDataNodeShowDetailsAction.cpp
  QmitkDataNodeShowSelectedNodesAction.cpp
  QmitkDataNodeSurfaceRepresentationAction.cpp
  QmitkDataNodeTextureInterpolationAction.cpp
  QmitkDataNodeToggleVisibilityAction.cpp
  QmitkDefaultDropTargetListener.cpp
  QmitkFileExitAction.cpp
  QmitkFileOpenAction.cpp
  QmitkFileSaveAction.cpp
  QmitkUndoAction.cpp
  QmitkRedoAction.cpp
  QmitkPreferencesDialog.cpp
  QmitkStatusBar.cpp
)

set(INTERNAL_CPP_FILES
  org_mitk_gui_qt_application_Activator.cpp
  QmitkEditorsPreferencePage.cpp
  QmitkGeneralPreferencePage.cpp
  QmitkShowPreferencePageHandler.cpp
)

set(MOC_H_FILES
  src/QmitkAbstractDataNodeAction.h
  src/QmitkCloseProjectAction.h
  src/QmitkDataNodeColorAction.h
  src/QmitkDataNodeColorMapAction.h
  src/QmitkDataNodeComponentAction.h
  src/QmitkDataNodeGlobalReinitAction.h
  src/QmitkDataNodeContextMenu.h
  src/QmitkDataNodeHideAllAction.h
  src/QmitkDataNodeOpacityAction.h
  src/QmitkDataNodeOpenInAction.h
  src/QmitkDataNodeReinitAction.h
  src/QmitkDataNodeRemoveAction.h
  src/QmitkDataNodeShowDetailsAction.h
  src/QmitkDataNodeShowSelectedNodesAction.h
  src/QmitkDataNodeSurfaceRepresentationAction.h
  src/QmitkDataNodeTextureInterpolationAction.h
  src/QmitkDataNodeToggleVisibilityAction.h
  src/QmitkFileExitAction.h
  src/QmitkFileOpenAction.h
  src/QmitkFileSaveAction.h
  src/QmitkUndoAction.h
  src/QmitkRedoAction.h
  src/QmitkPreferencesDialog.h

  src/internal/org_mitk_gui_qt_application_Activator.h
  src/internal/QmitkEditorsPreferencePage.h
  src/internal/QmitkGeneralPreferencePage.h
  src/internal/QmitkShowPreferencePageHandler.h
)

set(UI_FILES
  src/QmitkPreferencesDialog.ui
)

set(CACHED_RESOURCE_FILES
  plugin.xml
)

set(QRC_FILES
  resources/resources.qrc
)

set(CPP_FILES )

foreach(file ${SRC_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})
