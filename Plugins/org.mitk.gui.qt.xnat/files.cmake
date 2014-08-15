set(SRC_CPP_FILES

)

set(INTERNAL_CPP_FILES
  org_mitk_gui_qt_xnatinterface_Activator.cpp
  QmitkXnatEditor.cpp
  QmitkXnatTreeBrowserView.cpp
  QmitkXnatObjectEditorInput.cpp
  QmitkXnatConnectionPreferencePage.cpp
  QmitkXnatSessionManager.cpp
)

set(UI_FILES
  src/internal/QmitkXnatEditorControls.ui
  src/internal/QmitkXnatTreeBrowserViewControls.ui
  src/internal/QmitkXnatConnectionPreferencePageControls.ui
)

set(MOC_H_FILES
  src/internal/org_mitk_gui_qt_xnatinterface_Activator.h
  src/internal/QmitkXnatEditor.h
  src/internal/QmitkXnatTreeBrowserView.h
  src/internal/QmitkXnatConnectionPreferencePage.h
)

# list of resource files which can be used by the plug-in
# system without loading the plug-ins shared library,
# for example the icon used in the menu and tabs for the
# plug-in views in the workbench
set(CACHED_RESOURCE_FILES
  resources/icon.xpm
  resources/xnat_treebrowser_icon.xpm
  plugin.xml
)

# list of Qt .qrc files which contain additional resources
# specific to this plugin
set(QRC_FILES
)

set(CPP_FILES )

foreach(file ${SRC_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})
