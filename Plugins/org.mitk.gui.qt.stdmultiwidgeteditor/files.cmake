set(SRC_CPP_FILES
  QmitkStdMultiWidgetEditor.cpp
)

set(INTERNAL_CPP_FILES
  org_mitk_gui_qt_stdmultiwidgeteditor_Activator.cpp
  QmitkStdMultiWidgetEditorPreferencePage.cpp
)

set(MOC_H_FILES
  src/QmitkStdMultiWidgetEditor.h

  src/internal/org_mitk_gui_qt_stdmultiwidgeteditor_Activator.h
  src/internal/QmitkStdMultiWidgetEditorPreferencePage.h
)

set(UI_FILES

)

set(CACHED_RESOURCE_FILES
  plugin.xml
)

set(QRC_FILES
)

set(CPP_FILES )

foreach(file ${SRC_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})
