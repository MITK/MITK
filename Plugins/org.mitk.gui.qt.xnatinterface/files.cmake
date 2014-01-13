set(SRC_CPP_FILES

)

set(INTERNAL_CPP_FILES
  org_mitk_gui_qt_xnatinterface_Activator.cpp
  QmitkXnatEditor.cpp
  QmitkXnatSimpleSearchView.cpp
  QmitkXnatTreeBrowserView.cpp
  QmitkXnatObjectEditorInput.cpp
)

set(UI_FILES
  src/internal/QmitkXnatEditorControls.ui
  src/internal/QmitkXnatSimpleSearchViewControls.ui
  src/internal/QmitkXnatTreeBrowserViewControls.ui
)

set(MOC_H_FILES
  src/internal/org_mitk_gui_qt_xnatinterface_Activator.h
  src/internal/QmitkXnatEditor.h
  src/internal/QmitkXnatSimpleSearchView.h
  src/internal/QmitkXnatTreeBrowserView.h
  src/internal/QmitkXnatObjectEditorInput.h
)

# list of resource files which can be used by the plug-in
# system without loading the plug-ins shared library,
# for example the icon used in the menu and tabs for the
# plug-in views in the workbench
set(CACHED_RESOURCE_FILES
  resources/icon.xpm
  plugin.xml
)

# list of Qt .qrc files which contain additional resources
# specific to this plugin
set(QRC_FILES
  src/internal/images.qrc
)

set(CPP_FILES )

foreach(file ${SRC_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})
