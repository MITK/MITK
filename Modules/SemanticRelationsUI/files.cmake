file(GLOB_RECURSE H_FILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/include/*")

set(CPP_FILES
  mitkModuleActivator.cpp
  QmitkAbstractSemanticRelationsStorageInspector.cpp
  QmitkAbstractSemanticRelationsStorageModel.cpp
  QmitkControlPointDialog.cpp
  QmitkLesionTextDialog.cpp
  QmitkLesionTreeItem.cpp
  QmitkLesionTreeModel.cpp
  QmitkPatientTableHeaderView.cpp
  QmitkPatientTableInspector.cpp
  QmitkPatientTableModel.cpp
  QmitkSemanticRelationsUIHelper.cpp
  QmitkTableItemThumbnailDelegate.cpp
)

set(MOC_H_FILES
 include/QmitkAbstractSemanticRelationsStorageInspector.h
 include/QmitkAbstractSemanticRelationsStorageModel.h
 include/QmitkControlPointDialog.h
 include/QmitkLesionTextDialog.h
 include/QmitkLesionTreeModel.h
 include/QmitkPatientTableHeaderView.h
 include/QmitkPatientTableInspector.h
 include/QmitkPatientTableModel.h
 include/QmitkTableItemThumbnailDelegate.h
)

set(UI_FILES
  src/QmitkPatientTableInspector.ui
)
