set(MITK_CONFIG_PACKAGES
  Qt5
  BLUEBERRY
)

set(MITK_BUILD_APP_FlowBench ON CACHE BOOL "Build MITK FlowBench" FORCE)

set(MITK_WHITELIST "FlowBenchSegmentation" CACHE STRING "MITK FlowBench Whitelist" FORCE)
	
set(MITK_CONFIG_PLUGINS
  org.mitk.gui.qt.stdmultiwidgeteditor
  org.mitk.gui.qt.imagenavigator
  org.mitk.gui.qt.properties
  org.mitk.gui.qt.multilabelsegmentation
  org.mitk.gui.qt.flowbench.segmentation
)

set(MITK_VTK_DEBUG_LEAKS OFF CACHE BOOL "Enable VTK Debug Leaks" FORCE)