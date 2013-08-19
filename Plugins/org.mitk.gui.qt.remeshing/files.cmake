set(SRC_CPP_FILES
)

set(INTERNAL_CPP_FILES
  org_mitk_gui_qt_remeshing_Activator.cpp
  QmitkRemeshingView.cpp
)

set(UI_FILES
  src/internal/QmitkRemeshingViewControls.ui
)

set(MOC_H_FILES
  src/internal/org_mitk_gui_qt_remeshing_Activator.h
  src/internal/QmitkRemeshingView.h
)

set(CACHED_RESOURCE_FILES
  resources/RemeshingIcon.png
  plugin.xml
)

set(QRC_FILES
  resources/Remeshing.qrc
)

set(CPP_FILES
)

foreach(file ${SRC_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/${file})
endforeach()

foreach(file ${INTERNAL_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach()
