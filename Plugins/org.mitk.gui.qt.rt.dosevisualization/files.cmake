set(SRC_CPP_FILES
)

set(INTERNAL_CPP_FILES
  org_mitk_gui_qt_rt_dosevisualization_Activator.cpp
  RTDoseVisualizer.cpp
  RTUIPreferencePage.cpp
  DoseVisualizationPreferencePage.cpp
  mitkDoseVisPreferenceHelper.cpp
  LoadingRTView.cpp
)

set(UI_FILES
  src/internal/RTDoseVisualizerControls.ui
  src/internal/DoseVisualizationPreferencePageControls.ui
  src/internal/RTUIPreferencePageControls.ui
  src/internal/LoadingRTViewControls.ui
)

set(MOC_H_FILES
  src/internal/org_mitk_gui_qt_rt_dosevisualization_Activator.h
  src/internal/RTDoseVisualizer.h
  src/internal/RTUIPreferencePage.h
  src/internal/DoseVisualizationPreferencePage.h
  src/internal/LoadingRTView.h
)

# list of resource files which can be used by the plug-in
# system without loading the plug-ins shared library,
# for example the icon used in the menu and tabs for the
# plug-in views in the workbench
set(CACHED_RESOURCE_FILES
  resources/iso.png
  resources/iso2.png
  plugin.xml
)

# list of Qt .qrc files which contain additional resources
# specific to this plugin
set(QRC_FILES

)

set(CPP_FILES )

foreach(file ${SRC_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})
