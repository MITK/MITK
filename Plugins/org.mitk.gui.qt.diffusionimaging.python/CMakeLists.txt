if(MITK_USE_Python)

# The project name must correspond to the directory name of your plug-in
# and must not contain periods.
project(org_mitk_gui_qt_diffusionimaging_python)

mitk_create_plugin(
  EXPORT_DIRECTIVE DIFFUSIONIMAGING_PREPROCESSING_EXPORT
  EXPORTED_INCLUDE_SUFFIXES src
  MODULE_DEPENDS MitkDiffusionCore MitkPython
  PACKAGE_DEPENDS Qt5|Network BetData
)

endif()
