project(org_mitk_example_gui_customviewer_views)

set(QT_USE_QTSQL 1)

include_directories(${CTK_INCLUDE_DIRS})

MACRO_CREATE_MITK_CTK_PLUGIN(
  EXPORT_DIRECTIVE org_mitk_example_gui_customviewer_views_EXPORT
  EXPORTED_INCLUDE_SUFFIXES src
  MODULE_DEPENDENCIES QmitkExt mitkDicomUI
  )
