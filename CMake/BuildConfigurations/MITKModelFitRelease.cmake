include(${CMAKE_CURRENT_LIST_DIR}/WorkbenchRelease.cmake)

set(MITK_CONFIG_PLUGINS ${MITK_CONFIG_PLUGINS}
  org.mitk.gui.qt.fit.genericfitting
  org.mitk.gui.qt.fit.inspector
  org.mitk.gui.qt.pharmacokinetics.concentration.mri
  org.mitk.gui.qt.pharmacokinetics.mri
  org.mitk.gui.qt.pharmacokinetics.pet
  org.mitk.gui.qt.pharmacokinetics.curvedescriptor
)
