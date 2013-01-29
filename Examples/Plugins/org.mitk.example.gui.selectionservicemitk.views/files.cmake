set(SRC_CPP_FILES
)

set(INTERNAL_CPP_FILES
  org_mitk_example_gui_selectionservicemitk_views_Activator.cpp
  ListenerViewMitk.cpp
  SelectionViewMitk.cpp
)

set(UI_FILES
  src/internal/ListenerViewMitkControls.ui
  src/internal/SelectionViewMitkControls.ui
)

set(MOC_H_FILES
  src/internal/org_mitk_example_gui_selectionservicemitk_views_Activator.h
  src/internal/ListenerViewMitk.h
  src/internal/SelectionViewMitk.h
)

set(CACHED_RESOURCE_FILES
  plugin.xml
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
