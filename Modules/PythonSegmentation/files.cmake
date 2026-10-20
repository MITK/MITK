set(CPP_FILES
  mitknnInteractiveEnums.cpp
  mitknnInteractiveInteractor.cpp
  mitknnInteractiveTool.cpp
  mitknnInteractiveVersion.cpp
  mitknnInteractiveBoundingBoxHelpers.cpp
  mitknnInteractiveRenderingHelpers.cpp
  mitkInitializationHook.cpp
  mitkPythonPackageVersion.cpp
  mitkPythonWorkerModules.cpp
  mitkTorchDevice.cpp
  mitkTotalSegmentatorTool.cpp
  mitkVoxTellTool.cpp
)

set(RESOURCE_FILES
  package_checks.py
  torch_device.py
  nnInteractive/nninteractive_worker.py
  TotalSegmentator/totalsegmentator_worker.py
  VoxTell/VoxTell.svg
  VoxTell/voxtell_bridge.py
  VoxTell/voxtell_hub_files.py
  VoxTell/voxtell_worker.py
)

set(NNINTERACTIVE_INTERACTION_TYPES
  Point
  Box
  Scribble
  Lasso
)

foreach(interaction_type IN LISTS NNINTERACTIVE_INTERACTION_TYPES)
  list(APPEND CPP_FILES
    mitknnInteractive${interaction_type}Interactor.cpp
  )

  list(APPEND RESOURCE_FILES
    nnInteractive/${interaction_type}.svg
    nnInteractive/Positive${interaction_type}Cursor.svg
    nnInteractive/Negative${interaction_type}Cursor.svg
  )
endforeach()
