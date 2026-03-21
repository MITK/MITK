file(GLOB_RECURSE H_FILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/include/*")

set(CPP_FILES
  Qmitk/QmitkFreeIsoDoseLevelWidget.cpp
  Qmitk/QmitkIsoDoseLevelSetModel.cpp
  Qmitk/QmitkDoseColorDelegate.cpp
  Qmitk/QmitkDoseValueDelegate.cpp
  Qmitk/QmitkDoseVisualStyleDelegate.cpp
  Helper/mitkRTUIConstants.cpp
)

set(UI_FILES
  src/Qmitk/QmitkFreeIsoDoseLevelWidget.ui
)

set(QRC_FILES
  resources/RTUI.qrc
)
