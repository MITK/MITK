set(SRC_CPP_FILES
  QmitkAbstractMultiWidgetEditor.cpp
  QmitkAbstractRenderEditor.cpp
  QmitkAbstractView.cpp
  QmitkDataNodeSelectionProvider.cpp
  QmitkDnDFrameWidget.cpp
  QmitkSelectionServiceConnector.cpp
  QmitkSliceNavigationListener.cpp
  QmitkMultiWidgetDecorationManager.cpp
)

set(INTERNAL_CPP_FILES
  QmitkCommonActivator.cpp
  QmitkDataNodeItemModel.cpp
  QmitkDataNodeSelection.cpp
  QmitkViewCoordinator.cpp
  QmitkNodeSelectionPreferencePage.cpp
)

set(UI_FILES
  src/internal/QmitkNodeSelectionPreferencePage.ui
)

set(MOC_H_FILES
  src/QmitkAbstractMultiWidgetEditor.h
  src/QmitkAbstractRenderEditor.h
  src/QmitkDnDFrameWidget.h
  src/QmitkSelectionServiceConnector.h
  src/QmitkSliceNavigationListener.h
  src/internal/QmitkCommonActivator.h
  src/internal/QmitkNodeSelectionPreferencePage.h
)

set(CACHED_RESOURCE_FILES
  plugin.xml
)

set(CPP_FILES )

foreach(file ${SRC_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})
