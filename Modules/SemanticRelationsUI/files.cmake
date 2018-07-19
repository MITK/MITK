file(GLOB_RECURSE H_FILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/include/*")

set(CPP_FILES
  mitkModuleActivator.cpp
  QmitkControlPointDialog.cpp
  QmitkLesionTextDialog.cpp
  QmitkPatientTableInspector.cpp
  QmitkPatientTableModel.cpp
  QmitkSelectNodeDialog.cpp
  QmitkSelectionWidget.cpp
  QmitkSemanticRelationsModel.cpp
  QmitkSemanticRelationsUIHelper.cpp
  QmitkSimpleDatamanagerWidget.cpp
)

set(MOC_H_FILES
 include/QmitkControlPointDialog.h
 include/QmitkLesionTextDialog.h
 include/QmitkPatientTableInspector.h
 include/QmitkPatientTableModel.h
 include/QmitkSelectNodeDialog.h
 include/QmitkSelectionWidget.h
 include/QmitkSemanticRelationsModel.h

 include/QmitkSimpleDatamanagerWidget.h
)

set(UI_FILES
  src/QmitkPatientTableInspector.ui
  src/QmitkSimpleDatamanagerWidgetControls.ui
)
