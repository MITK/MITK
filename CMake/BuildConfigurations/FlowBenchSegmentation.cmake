set(MITK_CONFIG_PACKAGES
  Qt5
  BLUEBERRY
)

set(MITK_BUILD_APP_FlowBench ON CACHE BOOL "Build MITK FlowBench" FORCE)
	
set(MITK_CONFIG_PLUGINS
  org.mitk.gui.qt.stdmultiwidgeteditor
  org.mitk.gui.qt.imagenavigator
  org.mitk.gui.qt.properties
  org.mitk.gui.qt.flow.segmentation
)
