file(GLOB_RECURSE H_FILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/include/*")

set(CPP_FILES
  mitkXnatSessionTracker.cpp
  QmitkXnatTreeModel.cpp
  QmitkXnatProjectWidget.cpp
  QmitkXnatSubjectWidget.cpp
  QmitkXnatExperimentWidget.cpp
  QmitkXnatCreateObjectDialog.cpp
)

set(MOC_H_FILES
 include/mitkXnatSessionTracker.h
 include/QmitkXnatTreeModel.h
 include/QmitkXnatProjectWidget.h
 include/QmitkXnatSubjectWidget.h
 include/QmitkXnatExperimentWidget.h
 include/QmitkXnatCreateObjectDialog.h

)

set(QRC_FILES
  resources/xnat.qrc
)


set(UI_FILES
  src/QmitkXnatProjectWidgetControls.ui
  src/QmitkXnatSubjectWidgetControls.ui
  src/QmitkXnatExperimentWidgetControls.ui
)
