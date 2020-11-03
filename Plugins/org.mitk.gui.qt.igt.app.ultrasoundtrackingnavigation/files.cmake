set(SRC_CPP_FILES

)

set(INTERNAL_CPP_FILES
  org_mbi_gui_qt_usnavigation_Activator.cpp
  #USNavigation.cpp //not functional anymore. Delete code?
  QmitkUltrasoundCalibration.cpp
  QmitkUSNavigationMarkerPlacement.cpp

  QmitkUSNavigationPerspective.cpp

  mitkUSTargetPlacementQualityCalculator.cpp
  QmitkUSZonesDataModel.cpp
  QmitkUSNavigationCalibrationsDataModel.cpp
  QmitkUSZoneManagementColorDialogDelegate.cpp
  QmitkUSNavigationCalibrationRemoveDelegate.cpp
  QmitkUSNavigationCalibrationUpdateDepthDelegate.cpp

  Interactors/mitkUSZonesInteractor.cpp
  Interactors/mitkUSPointMarkInteractor.cpp

  Widgets/QmitkUSCombinedModalityCreationWidget.cpp
  Widgets/QmitkUSCombinedModalityEditWidget.cpp
  Widgets/QmitkUSNavigationFreezeButton.cpp
  Widgets/QmitkUSNavigationZoneDistancesWidget.cpp
  Widgets/QmitkUSZoneManagementWidget.cpp
  Widgets/QmitkZoneProgressBar.cpp

  NavigationStepWidgets/QmitkUSAbstractNavigationStep.cpp
  NavigationStepWidgets/QmitkUSNavigationStepCombinedModality.cpp
  NavigationStepWidgets/QmitkUSNavigationStepTumourSelection.cpp
  NavigationStepWidgets/QmitkUSNavigationStepZoneMarking.cpp
  NavigationStepWidgets/QmitkUSNavigationStepPlacementPlanning.cpp
  NavigationStepWidgets/QmitkUSNavigationStepMarkerIntervention.cpp
  NavigationStepWidgets/QmitkUSNavigationStepPunctuationIntervention.cpp
  NavigationStepWidgets/QmitkUSNavigationStepCtUsRegistration.cpp

  SettingsWidgets/QmitkUSNavigationCombinedSettingsWidget.cpp
  SettingsWidgets/QmitkUSNavigationAbstractSettingsWidget.cpp

  Filter/mitkUSNavigationTargetOcclusionFilter.cpp
  Filter/mitkUSNavigationTargetUpdateFilter.cpp
  Filter/mitkUSNavigationTargetIntersectionFilter.cpp
  Filter/mitkFloatingImageToUltrasoundRegistrationFilter.cpp

  IO/mitkUSNavigationCombinedModalityPersistence.cpp
  IO/mitkUSNavigationLoggingBackend.cpp
  IO/mitkUSNavigationExperimentLogging.cpp
  IO/mitkUSNavigationStepTimer.cpp
)

set(UI_FILES
  #src/internal/USNavigationControls.ui //not functional anymore. Delete code?
  src/internal/QmitkUltrasoundCalibrationControls.ui
  src/internal/QmitkUSNavigationMarkerPlacement.ui

  src/internal/Widgets/QmitkUSCombinedModalityCreationWidget.ui
  src/internal/Widgets/QmitkUSCombinedModalityEditWidget.ui
  src/internal/Widgets/QmitkUSZoneManagementWidget.ui

  src/internal/NavigationStepWidgets/QmitkUSAbstractNavigationStep.ui
  src/internal/NavigationStepWidgets/QmitkUSNavigationStepCombinedModality.ui
  src/internal/NavigationStepWidgets/QmitkUSNavigationStepTumourSelection.ui
  src/internal/NavigationStepWidgets/QmitkUSNavigationStepZoneMarking.ui
  src/internal/NavigationStepWidgets/QmitkUSNavigationStepPlacementPlanning.ui
  src/internal/NavigationStepWidgets/QmitkUSNavigationStepMarkerIntervention.ui
  src/internal/NavigationStepWidgets/QmitkUSNavigationStepPunctuationIntervention.ui
  src/internal/NavigationStepWidgets/QmitkUSNavigationStepCtUsRegistration.ui

  src/internal/SettingsWidgets/QmitkUSNavigationCombinedSettingsWidget.ui
)

set(MOC_H_FILES
  src/internal/org_mbi_gui_qt_usnavigation_Activator.h
  src/internal/QmitkUltrasoundCalibration.h
  src/internal/QmitkUSNavigationMarkerPlacement.h
  src/internal/QmitkUSZonesDataModel.h
  src/internal/QmitkUSNavigationCalibrationsDataModel.h
  src/internal/QmitkUSZoneManagementColorDialogDelegate.h
  src/internal/QmitkUSNavigationCalibrationRemoveDelegate.h
  src/internal/QmitkUSNavigationCalibrationUpdateDepthDelegate.h

  src/internal/QmitkUSNavigationPerspective.h

  src/internal/Widgets/QmitkUSCombinedModalityCreationWidget.h
  src/internal/Widgets/QmitkUSCombinedModalityEditWidget.h
  src/internal/Widgets/QmitkUSNavigationFreezeButton.h
  src/internal/Widgets/QmitkUSNavigationZoneDistancesWidget.h
  src/internal/Widgets/QmitkUSZoneManagementWidget.h
  src/internal/Widgets/QmitkZoneProgressBar.h

  src/internal/NavigationStepWidgets/QmitkUSAbstractNavigationStep.h
  src/internal/NavigationStepWidgets/QmitkUSNavigationStepCombinedModality.h
  src/internal/NavigationStepWidgets/QmitkUSNavigationStepTumourSelection.h
  src/internal/NavigationStepWidgets/QmitkUSNavigationStepZoneMarking.h
  src/internal/NavigationStepWidgets/QmitkUSNavigationStepPlacementPlanning.h
  src/internal/NavigationStepWidgets/QmitkUSNavigationStepMarkerIntervention.h
  src/internal/NavigationStepWidgets/QmitkUSNavigationStepPunctuationIntervention.h
  src/internal/NavigationStepWidgets/QmitkUSNavigationStepCtUsRegistration.h

  src/internal/SettingsWidgets/QmitkUSNavigationCombinedSettingsWidget.h
  src/internal/SettingsWidgets/QmitkUSNavigationAbstractSettingsWidget.h
)

# list of resource files which can be used by the plug-in
# system without loading the plug-ins shared library,
# for example the icon used in the menu and tabs for the
# plug-in views in the workbench
set(CACHED_RESOURCE_FILES
  resources/icon_US_navigation.svg
  resources/icon_US_calibration.svg
  plugin.xml
)

# list of Qt .qrc files which contain additional resources
# specific to this plugin
set(QRC_FILES
  resources/USNavigation.qrc
)

set(CPP_FILES )

foreach(file ${SRC_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})

set(RESOURCE_FILES
Interactions/USPointMarkInteractions.xml
Interactions/USZoneInteractions.xml
Interactions/USZoneInteractionsHold.xml
)
