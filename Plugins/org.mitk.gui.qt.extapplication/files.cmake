set(SRC_CPP_FILES
)

set(INTERNAL_CPP_FILES
  QmitkExtApplication.cpp
  QmitkExtApplicationPlugin.cpp
  QmitkExtAppWorkbenchAdvisor.cpp
  QmitkMitkWorkbenchIntroPart.cpp
  perspectives/QmitkEditorPerspective.cpp
  perspectives/QmitkExtDefaultPerspective.cpp
  perspectives/QmitkVisualizationPerspective.cpp
)

set(MOC_H_FILES
  src/internal/QmitkExtApplication.h
  src/internal/QmitkExtApplicationPlugin.h
  src/internal/QmitkMitkWorkbenchIntroPart.h
  src/internal/perspectives/QmitkEditorPerspective.h
  src/internal/perspectives/QmitkExtDefaultPerspective.h
  src/internal/perspectives/QmitkVisualizationPerspective.h
)

set(UI_FILES
  src/internal/perspectives/QmitkWelcomeScreenViewControls.ui
)

set(CACHED_RESOURCE_FILES
# list of resource files which can be used by the plug-in
# system without loading the plug-ins shared library,
# for example the icon used in the menu and tabs for the
# plug-in views in the workbench
  plugin.xml
  resources/icon_research.xpm
  resources/perspectives/eye.png
)

set(QRC_FILES
resources/QmitkExtApplication.qrc
resources/welcome/QmitkWelcomeScreenView.qrc
)

set(CPP_FILES )

foreach(file ${SRC_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})

