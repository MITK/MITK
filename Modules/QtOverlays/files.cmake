file(GLOB_RECURSE H_FILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/include/*")

set(CPP_FILES
  QmitkCustomWidgetOverlay.cpp
  QmitkOverlay.cpp
  QmitkOverlayContainerWidget.cpp
  QmitkOverlayController.cpp
  QmitkScalarBar.cpp
  QmitkScalarBarOverlay.cpp
  QmitkTextOverlay.cpp
)
