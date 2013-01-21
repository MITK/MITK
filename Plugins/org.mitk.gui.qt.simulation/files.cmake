set(SRC_CPP_FILES
)

set(INTERNAL_CPP_FILES
  org_mitk_gui_qt_simulation_Activator.cpp
  QmitkSimulationView.cpp
)

set(UI_FILES
  src/internal/QmitkSimulationViewControls.ui
)

set(MOC_H_FILES
  src/internal/org_mitk_gui_qt_simulation_Activator.h
  src/internal/QmitkSimulationView.h
)

set(CACHED_RESOURCE_FILES
  resources/icon.png
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
