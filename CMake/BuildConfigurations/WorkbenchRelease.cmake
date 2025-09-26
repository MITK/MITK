include(${CMAKE_CURRENT_LIST_DIR}/WorkbenchReleaseNoPython.cmake)

set(MITK_CONFIG_PACKAGES ${MITK_CONFIG_PACKAGES}
  SWIG
  Python3
)

set(MITK_CONFIG_PLUGINS ${MITK_CONFIG_PLUGINS}
  org.mitk.gui.qt.pythonsettings
)
