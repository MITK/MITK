project(org_blueberry_ui_qt_objectinspector)

mitk_create_plugin(
  EXPORT_DIRECTIVE BERRY_OBJECTINSPECTOR_EXPORT
  EXPORTED_INCLUDE_SUFFIXES src
)

if(NOT BLUEBERRY_DEBUG_SMARTPOINTER)
  message("You should set BLUEBERRY_DEBUG_SMARTPOINTER to ON if you want the plug-in ${Plugin-SymbolicName} to be useful")
endif()
