set(SRC_CPP_FILES
  QmitkMxNMultiWidgetEditor.cpp
  QmitkMultiWidgetDecorationManager.cpp
)

set(INTERNAL_CPP_FILES
  mitkPluginActivator.cpp
  QmitkMxNMultiWidgetEditorPreferencePage.cpp
)

set(UI_FILES
  src/internal/QmitkMxNMultiWidgetEditorPreferencePage.ui
)

set(MOC_H_FILES
  src/QmitkMxNMultiWidgetEditor.h
  
  src/internal/mitkPluginActivator.h
  src/internal/QmitkMxNMultiWidgetEditorPreferencePage.h
)

set(CACHED_RESOURCE_FILES
  resources/MxNMultiWidgetEditor.svg
  plugin.xml
)

set(QRC_FILES
  resources/QmitkMxNMultiWidgetEditor.qrc
)

foreach(file ${SRC_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})
