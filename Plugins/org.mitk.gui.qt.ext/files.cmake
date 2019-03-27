set(SRC_CPP_FILES
  QmitkExtActionBarAdvisor.cpp
  QmitkExtWorkbenchWindowAdvisor.cpp
  QmitkExtFileSaveProjectAction.cpp
  QmitkOpenDicomEditorAction.cpp
  QmitkOpenMxNMultiWidgetEditorAction.cpp
  QmitkOpenStdMultiWidgetEditorAction.cpp
)

set(INTERNAL_CPP_FILES
  QmitkAboutHandler.cpp
  QmitkAppInstancesPreferencePage.cpp
  QmitkExternalProgramsPreferencePage.cpp
  QmitkCommonExtPlugin.cpp
  QmitkInputDevicesPrefPage.cpp
  QmitkModuleView.cpp
)

set(UI_FILES
  src/internal/QmitkAppInstancesPreferencePage.ui
  src/internal/QmitkExternalProgramsPreferencePage.ui
)

set(MOC_H_FILES
  src/QmitkExtFileSaveProjectAction.h
  src/QmitkExtWorkbenchWindowAdvisor.h

  src/internal/QmitkAboutHandler.h
  src/internal/QmitkAppInstancesPreferencePage.h
  src/internal/QmitkExternalProgramsPreferencePage.h
  src/internal/QmitkCommonExtPlugin.h
  src/internal/QmitkExtWorkbenchWindowAdvisorHack.h
  src/internal/QmitkInputDevicesPrefPage.h
  src/internal/QmitkModuleView.h
  src/QmitkOpenDicomEditorAction.h
  src/QmitkOpenMxNMultiWidgetEditorAction.h
  src/QmitkOpenStdMultiWidgetEditorAction.h
)

set(CACHED_RESOURCE_FILES
# list of resource files which can be used by the plug-in
# system without loading the plug-ins shared library,
# for example the icon used in the menu and tabs for the
# plug-in views in the workbench
  plugin.xml
  resources/ModuleView.png
)

set(QRC_FILES
# uncomment the following line if you want to use Qt resources
 resources/org_mitk_gui_qt_ext.qrc
 resources/org_mitk_icons.qrc
)

set(CPP_FILES )

foreach(file ${SRC_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})
