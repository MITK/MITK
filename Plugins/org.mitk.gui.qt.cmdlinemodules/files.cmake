set(SRC_CPP_FILES
  QmitkCmdLineModuleMenuComboBox.cpp
)

set(INTERNAL_CPP_FILES
  QmitkDataStorageComboBoxWithSelectNone.cpp
  QmitkDirectoryListWidget.cpp
  QmitkFileListWidget.cpp
  QmitkCmdLineModuleGui.cpp
  QmitkCmdLineModuleFactoryGui.cpp
  QmitkUiLoader.cpp
  org_mitk_gui_qt_cmdlinemodules_Activator.cpp
  CommandLineModulesViewConstants.cpp
  CommandLineModulesViewControls.cpp
  CommandLineModulesPreferencesPage.cpp
  CommandLineModulesView.cpp
  QmitkCmdLineModuleRunner.cpp
)

set(UI_FILES
  src/internal/QmitkPathListWidget.ui
  src/internal/CommandLineModulesViewControls.ui
  src/internal/QmitkCmdLineModuleProgressWidget.ui
)

set(MOC_H_FILES
  src/QmitkCmdLineModuleMenuComboBox.h
  src/internal/QmitkDataStorageComboBoxWithSelectNone.h
  src/internal/QmitkDirectoryListWidget.h
  src/internal/QmitkFileListWidget.h
  src/internal/QmitkCmdLineModuleGui.h
  src/internal/QmitkUiLoader.h
  src/internal/org_mitk_gui_qt_cmdlinemodules_Activator.h
  src/internal/CommandLineModulesViewControls.h
  src/internal/CommandLineModulesPreferencesPage.h
  src/internal/CommandLineModulesView.h
  src/internal/QmitkCmdLineModuleRunner.h
)

# list of resource files which can be used by the plug-in
# system without loading the plug-ins shared library,
# for example the icon used in the menu and tabs for the
# plug-in views in the workbench
set(CACHED_RESOURCE_FILES
  resources/icon.xpm
  resources/run.png
  resources/stop.png
  plugin.xml
)

# list of Qt .qrc files which contain additional resources
# specific to this plugin
set(QRC_FILES
  resources/CommandLineModulesResources.qrc
)

set(CPP_FILES )

foreach(file ${SRC_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})
