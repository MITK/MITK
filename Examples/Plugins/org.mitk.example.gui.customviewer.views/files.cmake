set(SRC_CPP_FILES
)

set(INTERNAL_CPP_FILES
  DicomView.cpp
  SimpleRenderWindowView.cpp
  org_mitk_example_gui_customviewer_views_Activator.cpp
)

set(MOC_H_FILES
  src/internal/DicomView.h
  src/internal/SimpleRenderWindowView.h
  src/internal/org_mitk_example_gui_customviewer_views_Activator.h
)

set(UI_FILES
  src/internal/QmitkDicomViewControls.ui
)

set(QRC_FILES
)

set(CACHED_RESOURCE_FILES
  plugin.xml
)

set(CPP_FILES )

foreach(file ${SRC_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})
