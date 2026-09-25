set(SRC_CPP_FILES
)

set(INTERNAL_CPP_FILES
  QmitkMitkWorkbenchIntroPlugin.cpp
  QmitkMitkWorkbenchIntroPart.cpp
  QmitkWelcomePalette.cpp
  QmitkWelcomePersonalizationPage.cpp
  QmitkWelcomeRecentDataPage.cpp
  QmitkWelcomeText.cpp
  QmitkWelcomeTipsPage.cpp
)

set(MOC_H_FILES
  src/internal/QmitkMitkWorkbenchIntroPlugin.h
  src/internal/QmitkMitkWorkbenchIntroPart.h
  src/internal/QmitkWelcomePersonalizationPage.h
  src/internal/QmitkWelcomeRecentDataPage.h
  src/internal/QmitkWelcomeTipsPage.h
)

set(UI_FILES
)

set(CACHED_RESOURCE_FILES
# list of resource files which can be used by the plug-in
# system without loading the plug-ins shared library,
# for example the icon used in the menu and tabs for the
# plug-in views in the workbench
  plugin.xml
)

set(QRC_FILES
resources/QmitkWelcomeScreenView.qrc
)

foreach(file ${SRC_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})
