set(SRC_CPP_FILES
  QmitkPAUSViewerView.cpp
)

set(INTERNAL_CPP_FILES
  org_mitk_gui_qt_photoacoustics_pausviewer_Activator.cpp
)

set(UI_FILES
  src/internal/QmitkPAUSViewerViewControls.ui
)

set(MOC_H_FILES
  src/QmitkPAUSViewerView.h
  src/internal/org_mitk_gui_qt_photoacoustics_pausviewer_Activator.h
)

set(CACHED_RESOURCE_FILES
  resources/iconPAUSViewer.svg
  plugin.xml
)

foreach(file ${SRC_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})
