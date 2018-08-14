file(GLOB_RECURSE H_FILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/include/*")

set(CPP_FILES
  mitkModuleActivator.cpp
  QmitkAbstractSemanticRelationsStorageModel.cpp
  QmitkControlPointDialog.cpp
  QmitkLesionTextDialog.cpp
  QmitkPatientTableInspector.cpp
  QmitkPatientTableModel.cpp
  QmitkSemanticRelationsModel.cpp
  QmitkSemanticRelationsUIHelper.cpp
)

set(MOC_H_FILES
 include/QmitkAbstractSemanticRelationsStorageInspector.h
 include/QmitkAbstractSemanticRelationsStorageModel.h
 include/QmitkControlPointDialog.h
 include/QmitkLesionTextDialog.h
 include/QmitkPatientTableInspector.h
 include/QmitkPatientTableModel.h
 include/QmitkSemanticRelationsModel.h
)

set(UI_FILES
  src/QmitkPatientTableInspector.ui
)
