set(SRC_CPP_FILES
)

set(INTERNAL_CPP_FILES
  org_mitk_example_gui_selectionservicemitk_Activator.cpp
  SelectionServiceMitk.cpp
  ExtendedPerspective.cpp
)

set(UI_FILES
)

set(MOC_H_FILES
  src/internal/org_mitk_example_gui_selectionservicemitk_Activator.h
  src/internal/SelectionServiceMitk.h
  src/internal/ExtendedPerspective.h
)

set(CACHED_RESOURCE_FILES
  plugin.xml
  resources/perspectiveIcon1.png
)

set(CPP_FILES )

foreach(file ${SRC_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})
