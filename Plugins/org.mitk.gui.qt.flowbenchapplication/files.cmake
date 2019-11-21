set(SRC_CPP_FILES
)

set(INTERNAL_CPP_FILES
  QmitkFlowBenchApplication.cpp
  QmitkFlowBenchApplicationPlugin.cpp
  QmitkFlowBenchApplicationWorkbenchAdvisor.cpp
  QmitkFlowBenchApplicationWorkbenchWindowAdvisor.cpp  
  perspectives/QmitkFlowBenchApplicationPerspective.cpp
  QmitkExtFileSaveProjectAction.cpp
  QmitkOpenMxNMultiWidgetEditorAction.cpp
  QmitkOpenStdMultiWidgetEditorAction.cpp
)

set(MOC_H_FILES
  src/internal/QmitkFlowBenchApplication.h
  src/internal/QmitkFlowBenchApplicationPlugin.h
  src/internal/perspectives/QmitkFlowBenchApplicationPerspective.h
  src/internal/QmitkFlowBenchApplicationWorkbenchWindowAdvisor.h  
  src/internal/QmitkFlowBenchApplicationWorkbenchWindowAdvisorHack.h
  src/internal/QmitkExtFileSaveProjectAction.h
  src/internal/QmitkOpenMxNMultiWidgetEditorAction.h
  src/internal/QmitkOpenStdMultiWidgetEditorAction.h
)

set(CACHED_RESOURCE_FILES
# list of resource files which can be used by the plug-in
# system without loading the plug-ins shared library,
# for example the icon used in the menu and tabs for the
# plug-in views in the workbench
  plugin.xml
  resources/icon_research.xpm
  resources/perspectives/eye.png
  resources/perspectives/viewer_icon.png
)

set(QRC_FILES
resources/QmitkFlowBenchApplication.qrc
)

set(CPP_FILES )

foreach(file ${SRC_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})

