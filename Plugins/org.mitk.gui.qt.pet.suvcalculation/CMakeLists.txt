project(org_mitk_gui_qt_pet_suvcalculation)

mitk_create_plugin(
    EXPORT_DIRECTIVE PETSUVCALCULATION_EXPORT
    EXPORTED_INCLUDE_SUFFIXES src
    MODULE_DEPENDS MitkQtWidgetsExt MitkPET
)
