set(SRC_CPP_FILES
)

set(INTERNAL_CPP_FILES
  multipleperspectivesActivator.cpp
  MultiplePerspectives.cpp
  MinimalPerspective.cpp
  ExtendedPerspective.cpp
  MinimalView.cpp
  SelectionView.cpp
)

set(UI_FILES
  src/internal/MinimalViewControls.ui
  src/internal/SelectionViewControls.ui
)

set(MOC_H_FILES
  src/internal/multipleperspectivesActivator.h
  src/internal/MultiplePerspectives.h
  src/internal/MinimalPerspective.h
  src/internal/ExtendedPerspective.h
  src/internal/MinimalView.h
  src/internal/SelectionView.h
)

set(CACHED_RESOURCE_FILES
  plugin.xml
  resources/testIcon.png
  resources/testIcon2.png
)

set(CPP_FILES )

foreach(file ${SRC_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})
