file(GLOB_RECURSE H_FILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/include/*")

set(CPP_FILES
  mitkXnatSessionTracker.cpp
  QmitkXnatTreeModel.cpp
  QmitkXnatUploadFromDataStorageDialog.cpp
)

set(MOC_H_FILES
 include/mitkXnatSessionTracker.h
 include/QmitkXnatTreeModel.h
 include/QmitkXnatUploadFromDataStorageDialog.h
)

set(UI_FILES
 src/QmitkXnatUploadFromDataStorageDialog.ui
)

set(QRC_FILES
  resources/xnat.qrc
)
