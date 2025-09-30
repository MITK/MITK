include(${CMAKE_CURRENT_LIST_DIR}/Default.cmake)

set(MITK_CONFIG_PACKAGES ${MITK_CONFIG_PACKAGES}
  MatchPoint
)

set(MITK_CONFIG_PLUGINS ${MITK_CONFIG_PLUGINS}
  org.mitk.matchpoint.core.helper
  org.mitk.gui.qt.matchpoint.algorithm.browser
  org.mitk.gui.qt.matchpoint.algorithm.control
  org.mitk.gui.qt.matchpoint.mapper
  org.mitk.gui.qt.matchpoint.framereg
  org.mitk.gui.qt.matchpoint.visualizer
  org.mitk.gui.qt.matchpoint.evaluator
  org.mitk.gui.qt.matchpoint.manipulator
  org.mitk.gui.qt.dicominspector
  org.mitk.gui.qt.fit.genericfitting
  org.mitk.gui.qt.fit.inspector
  org.mitk.gui.qt.pharmacokinetics.mri
  org.mitk.gui.qt.pharmacokinetics.concentration.mri
  org.mitk.gui.qt.pharmacokinetics.curvedescriptor
)

if(NOT MITK_USE_SUPERBUILD)
  set(BUILD_CoreCmdApps ON CACHE BOOL "" FORCE)
  set(BUILD_MatchPointCmdApps ON CACHE BOOL "" FORCE)
  set(BUILD_SegmentationCmdApps ON CACHE BOOL "" FORCE)
  set(BUILD_DICOMCmdApps ON CACHE BOOL "" FORCE)
  set(BUILD_ModelFitMiniApps ON CACHE BOOL "" FORCE)
endif()

set(MITK_VTK_DEBUG_LEAKS OFF CACHE BOOL "Enable VTK Debug Leaks" FORCE)

find_package(Doxygen REQUIRED)

# Ensure that the in-application help can be build
set(BLUEBERRY_QT_HELP_REQUIRED ON CACHE BOOL "Required Qt help documentation in plug-ins" FORCE)

# Emsure that python is realy not configured (can happen if one switches config back and forth)
list(REMOVE_ITEM my_list SWIG Python3)
list(REMOVE_ITEM MITK_CONFIG_PLUGINS org.mitk.gui.qt.pythonsettings)
if(DEFINED MITK_USE_Python3 AND MITK_USE_Python3)
    message(STATUS "MITK_USE_Python3 is set to TRUE, remove it as we have selected a NoPython configuration")
    unset(MITK_USE_Python3 CACHE)
endif()
if(DEFINED MITK_USE_SWIG AND MITK_USE_SWIG)
    message(STATUS "MITK_USE_SWIG is set to TRUE, remove it as we have selected a NoPython configuration")
    unset(MITK_USE_SWIG CACHE)
endif()