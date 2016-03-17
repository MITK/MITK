SET(SRC_CPP_FILES
)

SET(INTERNAL_CPP_FILES
  mitkQmlAppPluginActivator.cpp
  QmitkQmlApplication.cpp
  QmitkQmlWorkbench.cpp
)

SET(MOC_H_FILES
  src/internal/QmitkQmlApplication.h
  src/internal/mitkQmlAppPluginActivator.h
  src/internal/QmitkQmlWorkbench.h
)

SET(CACHED_RESOURCE_FILES
# list of resource files which can be used by the plug-in
# system without loading the plug-ins shared library,
# for example the icon used in the menu and tabs for the
# plug-in views in the workbench
  plugin.xml
)

SET(QRC_FILES
# uncomment the following line if you want to use Qt resources
  resources/org_mitk_example_gui_qmlapplication.qrc
)

SET(CPP_FILES )

foreach(file ${SRC_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})
