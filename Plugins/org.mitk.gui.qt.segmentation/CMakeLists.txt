project(org_mitk_gui_qt_segmentation)

include_directories(${CTK_INCLUDE_DIRS})

set(MODULE_DEPENDS
  MitkAppUtil
  MitkQtWidgetsExt
  MitkSegmentationUI
)

if(TARGET MitkPythonSegmentationUI)
  list(APPEND MODULE_DEPENDS
    MitkPythonSegmentationUI
  )

  set(MITK_HAS_PYTHON 1)
else()
  set(MITK_HAS_PYTHON 0)
endif()

configure_file(
  mitkSegmentationPluginConfig.h.in
  mitkSegmentationPluginConfig.h
)

mitk_create_plugin(
  EXPORT_DIRECTIVE MITK_QT_SEGMENTATION
  EXPORTED_INCLUDE_SUFFIXES src
  PCH
    berryWeakPointer.h
    mitkDataNode.h
  MODULE_DEPENDS
    ${MODULE_DEPENDS}
  PACKAGE_DEPENDS
    Qt6|OpenGL
    nlohmann_json
)
