set(H_FILES
  include/mitkXnatSession.h
)

set(CPP_FILES
  mitkXnatSessionTracker.cpp
  QmitkXnatTreeModel.cpp
  QmitkXnatProjectWidget.cpp
  QmitkXnatSubjectWidget.cpp
  QmitkXnatExperimentWidget.cpp
  QmitkXnatCreateObjectDialog.cpp
  QmitkSelectXnatUploadDestinationDialog.cpp
  QmitkXnatUploadFromDataStorageDialog.cpp
  QmitkHttpStatusCodeHandler.cpp
)

set(QRC_FILES
  resources/xnat.qrc
)

set(UI_FILES
  src/QmitkXnatExperimentWidgetControls.ui
  src/QmitkXnatProjectWidgetControls.ui
  src/QmitkSelectXnatUploadDestinationDialog.ui
  src/QmitkXnatSubjectWidgetControls.ui
  src/QmitkXnatUploadFromDataStorageDialog.ui
)
