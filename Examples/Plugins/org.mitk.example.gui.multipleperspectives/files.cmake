set(SRC_CPP_FILES
)

set(INTERNAL_CPP_FILES
  org_mitk_example_gui_multipleperspectives_Activator.cpp
  MultiplePerspectives.cpp
  MinimalPerspective.cpp
  ExtendedPerspective.cpp
  EmptyView1.cpp
  EmptyView2.cpp
)

set(UI_FILES
  src/internal/EmptyView1Controls.ui
  src/internal/EmptyView2Controls.ui
)

set(MOC_H_FILES
  src/internal/org_mitk_example_gui_multipleperspectives_Activator.h
  src/internal/MultiplePerspectives.h
  src/internal/MinimalPerspective.h
  src/internal/ExtendedPerspective.h
  src/internal/EmptyView1.h
  src/internal/EmptyView2.h
)

set(CACHED_RESOURCE_FILES
  plugin.xml
  resources/perspectiveIcon1.png
  resources/perspectiveIcon2.png
  resources/viewIcon1.png
  resources/viewIcon2.png
)

set(CPP_FILES )

foreach(file ${SRC_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})
