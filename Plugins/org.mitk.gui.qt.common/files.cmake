set(SRC_CPP_FILES
  QmitkAbstractRenderEditor.cpp
  QmitkAbstractView.cpp
  QmitkDataNodeSelectionProvider.cpp
  QmitkDnDFrameWidget.cpp
  QmitkSelectionServiceConnector.cpp
  QmitkSliceNavigationListener.cpp
  QmitkSingleNodeSelectionWidget.cpp
  QmitkNodeSelectionDialog.cpp
  QmitkAbstractNodeSelectionWidget.cpp
  QmitkMultiNodeSelectionWidget.cpp
)

set(INTERNAL_CPP_FILES
  QmitkCommonActivator.cpp
  QmitkDataNodeItemModel.cpp
  QmitkDataNodeSelection.cpp
  QmitkViewCoordinator.cpp
)

set(UI_FILES
  src/QmitkSingleNodeSelectionWidget.ui
  src/QmitkMultiNodeSelectionWidget.ui
  src/QmitkNodeSelectionDialog.ui
)

set(MOC_H_FILES
  src/QmitkAbstractRenderEditor.h
  src/QmitkDnDFrameWidget.h
  src/QmitkSelectionServiceConnector.h
  src/QmitkSliceNavigationListener.h
  src/QmitkSingleNodeSelectionWidget.h
  src/QmitkNodeSelectionDialog.h
  src/QmitkAbstractNodeSelectionWidget.h
  src/QmitkMultiNodeSelectionWidget.h
  src/internal/QmitkCommonActivator.h
)

set(CACHED_RESOURCE_FILES
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
