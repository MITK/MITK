set(SRC_CPP_FILES
)

set(INTERNAL_CPP_FILES
  QmitkMitkWorkbenchIntroPlugin.cpp
  QmitkMitkWorkbenchIntroPart.cpp
)

set(MOC_H_FILES
  src/internal/QmitkMitkWorkbenchIntroPlugin.h
  src/internal/QmitkMitkWorkbenchIntroPart.h
)

set(UI_FILES
  src/internal/QmitkWelcomeScreenViewControls.ui
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

set(CPP_FILES )

foreach(file ${SRC_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})

