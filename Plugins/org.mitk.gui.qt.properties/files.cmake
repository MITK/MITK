set(SRC_CPP_FILES
)

set(INTERNAL_CPP_FILES
  org_mitk_gui_qt_properties_Activator.cpp
  QmitkPropertyItemSortFilterProxyModel.cpp
  QmitkPropertyTreeView.cpp
  QmitkAddNewPropertyDialog.cpp
)

set(UI_FILES
  src/internal/QmitkPropertyTreeView.ui
  src/internal/QmitkAddNewPropertyDialog.ui
)

set(MOC_H_FILES
  src/internal/org_mitk_gui_qt_properties_Activator.h
  src/internal/QmitkPropertyItemSortFilterProxyModel.h
  src/internal/QmitkPropertyTreeView.h
  src/internal/QmitkAddNewPropertyDialog.h
)

set(CACHED_RESOURCE_FILES
  resources/property_list.svg
  plugin.xml
)

set(QRC_FILES
  resources/Properties.qrc
)

set(CPP_FILES
)

foreach(file ${SRC_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/${file})
endforeach()

foreach(file ${INTERNAL_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach()
