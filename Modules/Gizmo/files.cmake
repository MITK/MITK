set(H_FILES
  include/mitkGizmo.h
  include/mitkGizmoInteractor3D.h
)

set(CPP_FILES
  mitkGizmo.cpp
  mitkGizmoInteractor3D.cpp
)

set(RESOURCE_FILES
  # "Interactions" prefix forced by mitk::StateMachine
  Interactions/Gizmo3DStates.xml
  Interactions/Gizmo3DConfig.xml
)
