set(SRC_CPP_FILES
)

set(INTERNAL_CPP_FILES
  org_mitk_example_gui_testapplication_Activator.cpp
  TestApplication.cpp
  TestPerspective.cpp
  TestPerspective2.cpp
  TestView.cpp
  MinimalView.cpp
  TestApplicationWorkbenchWindowAdvisor.cpp
  QtPerspectiveSwitcherTabBar.cpp
)

set(MOC_H_FILES
  src/internal/org_mitk_example_gui_testapplication_Activator.h
  src/internal/TestApplication.h
  src/internal/TestPerspective.h
  src/internal/TestPerspective2.h
  src/internal/TestView.h
  src/internal/MinimalView.h
  src/internal/TestApplicationWorkbenchWindowAdvisor.h
  src/internal/QtPerspectiveSwitcherTabBar.h
)

set(UI_FILES
  src/internal/QmitkMinimalViewControls.ui
  src/internal/QmitkTestView1.ui
  src/internal/QmitkTestViewControls.ui
)

set(QRC_FILES
  resources/org_mitk_example_gui_testapplication.qrc
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
