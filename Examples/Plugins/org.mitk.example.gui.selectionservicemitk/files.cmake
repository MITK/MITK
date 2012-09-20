set(SRC_CPP_FILES
)

set(INTERNAL_CPP_FILES
  selectionservicemitkActivator.cpp
  SelectionServiceMITK.cpp
  ExtendedPerspective.cpp
  ListenerViewMitk.cpp
  SelectionViewMitk.cpp
)

set(UI_FILES
  src/internal/ListenerViewMitkControls.ui
  src/internal/SelectionViewMitkControls.ui
)

set(MOC_H_FILES
  src/internal/selectionservicemitkActivator.h
  src/internal/SelectionServiceMITK.h
  src/internal/ExtendedPerspective.h
  src/internal/ListenerViewMitk.h
  src/internal/SelectionViewMitk.h
)

set(CACHED_RESOURCE_FILES
  plugin.xml
  resources/perspectiveIcon1.png
  resources/selectionListenerIcon.png
  resources/selectionProviderIcon.png
)

set(CPP_FILES )

foreach(file ${SRC_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})
