file(GLOB_RECURSE H_FILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/include/*")

set(CPP_FILES
  QmitkControlPointDialog.cpp
  QmitkPatientInfoWidget.cpp
  QmitkPatientTableModel.cpp
  QmitkPatientTableWidget.cpp
  QmitkLesionInfoWidget.cpp
  QmitkSelectNodeDialog.cpp
  QmitkSelectionWidget.cpp
  QmitkSimpleDatamanagerWidget.cpp
)

set(MOC_H_FILES
 include/QmitkControlPointDialog.h
 include/QmitkPatientInfoWidget.h
 include/QmitkPatientTableModel.h
 include/QmitkPatientTableWidget.h
 include/QmitkLesionInfoWidget.h
 include/QmitkSelectNodeDialog.h
 include/QmitkSelectionWidget.h
 include/QmitkSimpleDatamanagerWidget.h
)

set(UI_FILES
  src/QmitkPatientInfoWidgetControls.ui
  src/QmitkPatientTableWidgetControls.ui
  src/QmitkLesionInfoWidgetControls.ui
  src/QmitkSimpleDatamanagerWidgetControls.ui
)
