set(SRC_CPP_FILES
  QmitkSingleNodeSelectionWidget.cpp
  QmitkNodeSelectionDialog.cpp
  QmitkAbstractNodeSelectionWidget.cpp
  QmitkMultiNodeSelectionWidget.cpp
)

set(INTERNAL_CPP_FILES
  mitkPluginActivator.cpp
  QmitkDataStorageViewerTestView.cpp
)

set(UI_FILES
  src/internal/QmitkDataStorageViewerTestControls.ui
  src/QmitkSingleNodeSelectionWidget.ui
  src/QmitkMultiNodeSelectionWidget.ui
  src/QmitkNodeSelectionDialog.ui
)

set(MOC_H_FILES
  src/internal/mitkPluginActivator.h
  src/internal/QmitkDataStorageViewerTestView.h
  src/QmitkSingleNodeSelectionWidget.h
  src/QmitkNodeSelectionDialog.h
  src/QmitkAbstractNodeSelectionWidget.h
  src/QmitkMultiNodeSelectionWidget.h
)

set(CACHED_RESOURCE_FILES
  resources/DataStorageViewer_48.png
  plugin.xml
)

set(QRC_FILES
)

foreach(file ${SRC_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})
