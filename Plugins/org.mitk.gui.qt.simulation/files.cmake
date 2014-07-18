set(SRC_CPP_FILES
)

set(INTERNAL_CPP_FILES
  org_mitk_gui_qt_simulation_Activator.cpp
  QmitkBaseItemDelegate.cpp
  QmitkBaseTreeWidget.cpp
  QmitkNoEditItemDelegate.cpp
  QmitkSceneTreeWidget.cpp
  QmitkSimulationPreferencePage.cpp
  QmitkSimulationView.cpp
)

set(UI_FILES
  src/internal/QmitkSimulationPreferencePageControls.ui
  src/internal/QmitkSimulationViewControls.ui
)

set(MOC_H_FILES
  src/internal/org_mitk_gui_qt_simulation_Activator.h
  src/internal/QmitkBaseItemDelegate.h
  src/internal/QmitkBaseTreeWidget.h
  src/internal/QmitkNoEditItemDelegate.h
  src/internal/QmitkSceneTreeWidget.h
  src/internal/QmitkSimulationPreferencePage.h
  src/internal/QmitkSimulationView.h
)

set(CACHED_RESOURCE_FILES
  resources/SOFAIcon.png
  plugin.xml
)

set(QRC_FILES
  resources/Simulation.qrc
)

set(CPP_FILES
)

foreach(file ${SRC_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/${file})
endforeach()

foreach(file ${INTERNAL_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach()
