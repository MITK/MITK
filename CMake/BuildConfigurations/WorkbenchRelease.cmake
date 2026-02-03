include(${CMAKE_CURRENT_LIST_DIR}/internal/WorkbenchReleaseBase.cmake)

set(MITK_CONFIG_PACKAGES ${MITK_CONFIG_PACKAGES}
  Python3
  pybind11
)

set(MITK_CONFIG_PLUGINS ${MITK_CONFIG_PLUGINS}
  org.mitk.gui.qt.pythonsettings
)
