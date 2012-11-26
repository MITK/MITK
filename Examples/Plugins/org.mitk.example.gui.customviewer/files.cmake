set(SRC_CPP_FILES
)

set(INTERNAL_CPP_FILES
  org_mitk_example_gui_customviewer_Activator.cpp
  CustomViewer.cpp
  CustomViewerWorkbenchAdvisor.cpp
  ViewerPerspective.cpp
  DicomPerspective.cpp
  CustomViewerWorkbenchWindowAdvisor.cpp
  QtPerspectiveSwitcherTabBar.cpp
)

set(MOC_H_FILES
  src/internal/org_mitk_example_gui_customviewer_Activator.h
  src/internal/CustomViewer.h
  src/internal/ViewerPerspective.h
  src/internal/DicomPerspective.h
  src/internal/CustomViewerWorkbenchWindowAdvisor.h
  src/internal/QtPerspectiveSwitcherTabBar.h
)

set(UI_FILES
)

set(QRC_FILES
  resources/org_mitk_example_gui_customviewer.qrc
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
