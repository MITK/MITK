set(SRC_CPP_FILES
  QmitkCustomMultiWidgetEditor.cpp
)

set(INTERNAL_CPP_FILES
  mitkPluginActivator.cpp
  QmitkCustomMultiWidgetEditorPreferencePage.cpp
)

set(UI_FILES
  src/internal/QmitkCustomMultiWidgetEditorPreferencePage.ui
)

set(MOC_H_FILES
  src/QmitkCustomMultiWidgetEditor.h
  
  src/internal/mitkPluginActivator.h
  src/internal/QmitkCustomMultiWidgetEditorPreferencePage.h
)

set(CACHED_RESOURCE_FILES
  resources/CustomMultiWidgetEditor.svg
  plugin.xml
)

set(QRC_FILES
)

foreach(file ${SRC_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})
