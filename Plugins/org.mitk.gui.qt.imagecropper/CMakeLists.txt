project(org_mitk_gui_qt_imagecropper)

mitk_create_plugin(
  EXPORT_DIRECTIVE MITK_QT_IMAGECROPPER
  EXPORTED_INCLUDE_SUFFIXES src
  MODULE_DEPENDS MitkQtWidgetsExt MitkBoundingShape MitkMultilabel
)

