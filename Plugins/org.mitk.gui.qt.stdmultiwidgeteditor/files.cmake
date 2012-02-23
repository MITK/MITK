SET(SRC_CPP_FILES
  QmitkStdMultiWidgetEditor.cpp
)

SET(INTERNAL_CPP_FILES
  org_mitk_gui_qt_stdmultiwidgeteditor_Activator.cpp
  QmitkStdMultiWidgetEditorPreferencePage.cpp
)

SET(MOC_H_FILES
  src/QmitkStdMultiWidgetEditor.h
  
  src/internal/org_mitk_gui_qt_stdmultiwidgeteditor_Activator.h
  src/internal/QmitkStdMultiWidgetEditorPreferencePage.h
)

SET(UI_FILES

)

SET(CACHED_RESOURCE_FILES
  plugin.xml
)

SET(QRC_FILES
)

SET(CPP_FILES )

foreach(file ${SRC_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})
