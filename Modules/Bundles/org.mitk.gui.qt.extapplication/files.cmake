SET(SRC_CPP_FILES
  QmitkExtApplication.cpp
  QmitkExtAppWorkbenchAdvisor.cpp
)

SET(INTERNAL_CPP_FILES
  QmitkExtApplicationPlugin.cpp
  QmitkExtDefaultPerspective.cpp
)

SET(MOC_H_FILES
  src/QmitkExtApplication.h
  src/internal/QmitkExtApplicationPlugin.h
  src/internal/QmitkExtDefaultPerspective.h
)

SET(CACHED_RESOURCE_FILES
# list of resource files which can be used by the plug-in
# system without loading the plug-ins shared library,
# for example the icon used in the menu and tabs for the
# plug-in views in the workbench
  plugin.xml
  resources/icon_research.xpm
)

SET(QRC_FILES
# uncomment the following line if you want to use Qt resources
#  resources/QmitkExtApplication.qrc
)

SET(CPP_FILES )

foreach(file ${SRC_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})

