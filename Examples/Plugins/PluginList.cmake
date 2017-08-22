
# Plug-ins must be ordered according to their dependencies

set(MITK_EXAMPLE_PLUGINS
  org.mitk.example.gui.minimalapplication:ON
  org.mitk.example.gui.customviewer:ON
  org.mitk.example.gui.customviewer.views:ON
  org.mitk.example.gui.multipleperspectives:ON
  org.mitk.example.gui.selectionserviceqt:ON
  org.mitk.example.gui.selectionservicemitk:ON
  org.mitk.example.gui.selectionservicemitk.views:ON
  org.mitk.example.gui.extensionpointdefinition:ON
  org.mitk.example.gui.extensionpointcontribution:ON
  org.mitk.example.gui.regiongrowing:ON
  org.mitk.example.gui.pcaexample:ON
  org.mitk.example.gui.imaging:ON
)

if(MITK_USE_OpenCV)
  list(APPEND MITK_EXAMPLE_PLUGINS
       org.mitk.example.gui.opencv:ON)
else()
  list(APPEND MITK_EXAMPLE_PLUGINS
       org.mitk.example.gui.opencv:OFF)
endif()
