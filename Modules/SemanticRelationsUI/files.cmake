file(GLOB_RECURSE H_FILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/include/*")

set(CPP_FILES
  QmitkPatientInfoWidget.cpp
  QmitkControlPointDialog.cpp
)

set(MOC_H_FILES
 include/QmitkPatientInfoWidget.h
 include/QmitkControlPointDialog.h
)

set(UI_FILES
  src/QmitkPatientInfoWidgetControls.ui
)
