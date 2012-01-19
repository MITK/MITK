SET(SRC_CPP_FILES
  QmitkCommonWorkbenchWindowAdvisor.cpp
  QmitkExtActionBarAdvisor.cpp
  QmitkExtWorkbenchWindowAdvisor.cpp
  QmitkExtFileOpenAction.cpp
  QmitkExtDnDFrameWidget.cpp
  QmitkExtFileSaveProjectAction.cpp
)

SET(INTERNAL_CPP_FILES
  QmitkAppInstancesPreferencePage.cpp
  QmitkCommonExtPlugin.cpp
  QmitkInputDevicesPrefPage.cpp
  QmitkModuleView.cpp
)

SET(UI_FILES
  src/internal/QmitkAppInstancesPreferencePage.ui
)

SET(MOC_H_FILES
  src/QmitkExtFileOpenAction.h
  src/QmitkExtFileSaveProjectAction.h
  src/QmitkExtDnDFrameWidget.h
  src/QmitkExtWorkbenchWindowAdvisor.h
  
  src/internal/QmitkAppInstancesPreferencePage.h
  src/internal/QmitkCommonExtPlugin.h
  src/internal/QmitkExtWorkbenchWindowAdvisorHack.h
  src/internal/QmitkInputDevicesPrefPage.h
  src/internal/QmitkModuleView.h
)

SET(CACHED_RESOURCE_FILES
# list of resource files which can be used by the plug-in
# system without loading the plug-ins shared library,
# for example the icon used in the menu and tabs for the
# plug-in views in the workbench
  plugin.xml
  resources/ModuleView.png
)

SET(QRC_FILES
# uncomment the following line if you want to use Qt resources
 resources/org_mitk_gui_qt_ext.qrc
)

SET(CPP_FILES )

foreach(file ${SRC_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})
