file(GLOB_RECURSE H_FILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/include/*")

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

set(MOC_H_FILES
 include/mitkXnatSessionTracker.h
 include/QmitkXnatTreeModel.h
 include/QmitkXnatProjectWidget.h
 include/QmitkXnatSubjectWidget.h
 include/QmitkXnatExperimentWidget.h
 include/QmitkXnatCreateObjectDialog.h
 include/QmitkSelectXnatUploadDestinationDialog.h
 include/QmitkXnatUploadFromDataStorageDialog.h
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
