set(SRC_CPP_FILES
  QmitkPAUSMultiWidgetEditor.cpp
)

set(INTERNAL_CPP_FILES
  org_mitk_gui_qt_pausmultiwidgeteditor_Activator.cpp
)

set(MOC_H_FILES
  src/QmitkPAUSMultiWidgetEditor.h
  src/internal/org_mitk_gui_qt_pausmultiwidgeteditor_Activator.h
)

set(CACHED_RESOURCE_FILES
  plugin.xml
  resources/PAUSMultiWidgetEditor.png
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
