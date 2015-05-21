file(GLOB_RECURSE H_FILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/include/*")

set(CPP_FILES
  mitkXnatSessionTracker.cpp
  QmitkXnatProjectInfoWidget.cpp
  QmitkXnatSubjectInfoWidget.cpp
  QmitkXnatExperimentInfoWidget.cpp
)

set(MOC_H_FILES
  include/mitkXnatSessionTracker.h
  include/QmitkXnatProjectInfoWidget.h
  include/QmitkXnatSubjectInfoWidget.h
  include/QmitkXnatExperimentInfoWidget.h
)

set(UI_FILES
  src/QmitkXnatProjectInfoWidgetControls.ui
  src/QmitkXnatSubjectInfoWidgetControls.ui
  src/QmitkXnatExperimentInfoWidgetControls.ui
)
