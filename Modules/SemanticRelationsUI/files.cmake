file(GLOB_RECURSE H_FILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/include/*")

set(CPP_FILES
  mitkModuleActivator.cpp
  QmitkAbstractSemanticRelationsStorageModel.cpp
  QmitkControlPointDialog.cpp
  QmitkLesionInfoWidget.cpp
  QmitkPatientTableInspector.cpp
  QmitkPatientTableModel.cpp
  QmitkSelectionWidget.cpp
  QmitkSelectNodeDialog.cpp
  QmitkSemanticRelationsUIHelper.cpp
  QmitkSimpleDatamanagerWidget.cpp
)

set(MOC_H_FILES
 include/QmitkAbstractSemanticRelationsStorageModel.h
 include/QmitkControlPointDialog.h
 include/QmitkLesionInfoWidget.h
 include/QmitkPatientTableInspector.h
 include/QmitkPatientTableModel.h
 include/QmitkSelectionWidget.h
 include/QmitkSelectNodeDialog.h
 include/QmitkSimpleDatamanagerWidget.h
)

set(UI_FILES
  src/QmitkLesionInfoWidgetControls.ui
  src/QmitkPatientTableInspector.ui
  src/QmitkSimpleDatamanagerWidgetControls.ui
)
