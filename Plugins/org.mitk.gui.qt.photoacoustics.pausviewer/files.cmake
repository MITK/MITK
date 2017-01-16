set(INTERNAL_CPP_FILES
  QmitkPAUSViewerView.cpp
  org_mitk_gui_qt_photoacoustics_pausviewer_Activator.cpp
)

set(UI_FILES
  src/internal/QmitkPAUSViewerViewControls.ui
)

set(MOC_H_FILES
  src/internal/QmitkPAUSViewerView.h
  src/internal/org_mitk_gui_qt_photoacoustics_pausviewer_Activator.h
)

set(CACHED_RESOURCE_FILES
  resources/PAUSViewer.png
  plugin.xml
)

foreach(file ${SRC_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})