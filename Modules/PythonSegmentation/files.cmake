set(CPP_FILES
  mitknnInteractiveEnums.cpp
  mitknnInteractiveInteractor.cpp
  mitknnInteractiveTool.cpp
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
