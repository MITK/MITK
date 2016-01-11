set(H_FILES
  src/mitkGizmoMapper2D.h
  src/mitkGizmoObjectFactory.h
  include/mitkGizmo.h
  include/mitkGizmoInteractor3D.h
)

set(CPP_FILES
  mitkGizmo.cpp
  mitkGizmoInteractor3D.cpp
  mitkGizmoObjectFactory.cpp
  mitkGizmoMapper2D.cpp
)

set(RESOURCE_FILES
  # "Interactions" prefix forced by mitk::StateMachine
  Interactions/Gizmo3DStates.xml
  Interactions/Gizmo3DConfig.xml
)
