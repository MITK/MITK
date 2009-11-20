SET(SRC_CPP_FILES
  QmitkExtApplication.cpp
  QmitkExtAppWorkbenchAdvisor.cpp
)

SET(INTERNAL_CPP_FILES
  QmitkExtDefaultPerspective.cpp
)

SET(MOC_H_FILES
 
)

SET(RESOURCE_FILES
# list of resource files which can be used by the plug-in
# system without loading the plug-ins shared library,
# for example the icon used in the menu and tabs for the
# plug-in views in the workbench
  resources/icon_research.xpm
)

SET(CPP_FILES manifest.cpp)

foreach(file ${SRC_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})
