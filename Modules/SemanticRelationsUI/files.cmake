file(GLOB_RECURSE H_FILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/include/*")

set(CPP_FILES
  QmitkPatientInfoWidget.cpp
  QmitkControlPointDialog.cpp
  QmitkSemanticRelationsDataModel.cpp
  QmitkSemanticRelationsTableView.cpp
)

set(MOC_H_FILES
 include/QmitkPatientInfoWidget.h
 include/QmitkControlPointDialog.h
 include/QmitkSemanticRelationsDataModel.h
 include/QmitkSemanticRelationsTableView.h
)

set(UI_FILES
  src/QmitkPatientInfoWidgetControls.ui
  src/QmitkSemanticRelationsTableViewControls.ui
)
