file(GLOB_RECURSE H_FILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/include/*")

set(CPP_FILES
  mitkXnatSessionTracker.cpp
  QmitkXnatTreeModel.cpp
)

set(MOC_H_FILES
 include/mitkXnatSessionTracker.h
 include/QmitkXnatTreeModel.h
)

set(QRC_FILES
  resources/xnat.qrc
)
