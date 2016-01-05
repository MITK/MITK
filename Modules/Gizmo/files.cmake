set(H_FILES
  src/mitkGizmoActivator.h
  include/mitkGizmo.h
  include/mitkGizmoInteractor3D.h
)

set(CPP_FILES
  mitkGizmoActivator.cpp
  mitkGizmo.cpp
  mitkGizmoInteractor3D.cpp
)

set(RESOURCE_FILES
  # "Interactions" prefix forced by mitk::StateMachine
  Interactions/Gizmo3DStates.xml
  Interactions/Gizmo3DConfig.xml
)
