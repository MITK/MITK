set(SRC_CPP_FILES
  mitkGetSimulationPreferences.cpp
  mitkMeshMitkLoader.cpp
)

set(INTERNAL_CPP_FILES
  org_mitk_simulation_Activator.cpp
)

set(MOC_H_FILES
  src/internal/org_mitk_simulation_Activator.h
)

set(CACHED_RESOURCE_FILES
  resources/SOFAIcon.png
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
