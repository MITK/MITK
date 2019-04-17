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
  QmitkNodeSelectionPreferenceHelper.cpp
  QmitkNodeSelectionButton.cpp
  QmitkSimpleTextOverlayWidget.cpp
)

set(INTERNAL_CPP_FILES
  QmitkCommonActivator.cpp
  QmitkDataNodeItemModel.cpp
  QmitkDataNodeSelection.cpp
  QmitkViewCoordinator.cpp
  QmitkNodeSelectionConstants.cpp
  QmitkNodeSelectionPreferencePage.cpp
  QmitkNodeSelectionListItemWidget.cpp
)

set(UI_FILES
  src/QmitkSingleNodeSelectionWidget.ui
  src/QmitkMultiNodeSelectionWidget.ui
  src/QmitkNodeSelectionDialog.ui
  src/internal/QmitkNodeSelectionPreferencePage.ui
  src/internal/QmitkNodeSelectionListItemWidget.ui
)

set(MOC_H_FILES
  src/QmitkAbstractRenderEditor.h
  src/QmitkDnDFrameWidget.h
  src/QmitkSelectionServiceConnector.h
  src/QmitkSliceNavigationListener.h
  src/ImporterUtil.h
  src/QmitkSingleNodeSelectionWidget.h
  src/QmitkNodeSelectionDialog.h
  src/QmitkAbstractNodeSelectionWidget.h
  src/QmitkMultiNodeSelectionWidget.h
  src/QmitkNodeSelectionButton.h
  src/QmitkSimpleTextOverlayWidget.h
  src/internal/QmitkCommonActivator.h
  src/internal/QmitkNodeSelectionPreferencePage.h
  src/internal/QmitkNodeSelectionListItemWidget.h
)

set(CACHED_RESOURCE_FILES
  plugin.xml
  resources/times.svg
)

set(QRC_FILES
  resources/common.qrc
)

set(CPP_FILES )

foreach(file ${SRC_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})
