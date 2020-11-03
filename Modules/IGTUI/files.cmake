set(CPP_FILES
  Qmitk/QmitkTrackingDeviceWidget.cpp
  Qmitk/QmitkTrackingDeviceConfigurationWidget.cpp
  Qmitk/QmitkNDIConfigurationWidget.cpp
  Qmitk/QmitkFiducialRegistrationWidget.cpp
  Qmitk/QmitkNDIToolDelegate.cpp
  Qmitk/QmitkNavigationToolManagementWidget.cpp
  Qmitk/QmitkIGTLoggerWidget.cpp
  Qmitk/QmitkUpdateTimerWidget.cpp
  Qmitk/QmitkToolDistanceWidget.cpp
  Qmitk/QmitkToolTrackingStatusWidget.cpp
  Qmitk/QmitkTrackingSourcesCheckBoxPanelWidget.cpp
  Qmitk/QmitkIGTPlayerWidget.cpp
  Qmitk/QmitkIGTConnectionWidget.cpp
  Qmitk/QmitkToolSelectionWidget.cpp
  Qmitk/QmitkNavigationToolCreationWidget.cpp
  Qmitk/QmitkNavigationDataSourceSelectionWidget.cpp
  Qmitk/QmitkInteractiveTransformationWidget.cpp
  Qmitk/QmitkNavigationToolStorageSelectionWidget.cpp

  Qmitk/QmitkNavigationDataPlayerControlWidget.cpp
  Qmitk/QmitkNavigationDataSequentialPlayerControlWidget.cpp

  Qmitk/QmitkNDIAuroraWidget.cpp
  Qmitk/QmitkNDIPolarisWidget.cpp
  Qmitk/QmitkMicronTrackerWidget.cpp
  Qmitk/QmitkNPOptitrackWidget.cpp
  Qmitk/QmitkVirtualTrackerWidget.cpp
  Qmitk/QmitkOpenIGTLinkWidget.cpp

  Qmitk/mitkTrackingDeviceWidgetCollection.cpp
  Qmitk/QmitkTrackingDeviceConfigurationWidgetScanPortsWorker.cpp
  Qmitk/QmitkTrackingDeviceConfigurationWidgetConnectionWorker.cpp
  Qmitk/QmitkNDIAbstractDeviceWidget.cpp
  Qmitk/QmitkAbstractTrackingDeviceWidget.cpp
  Qmitk/QmitkIGTCommonHelper.cpp
)

set(UI_FILES
  Qmitk/QmitkNavigationToolManagementWidgetControls.ui
  Qmitk/QmitkTrackingDeviceConfigurationWidgetControls.ui
  Qmitk/QmitkNDIConfigurationWidget.ui
  Qmitk/QmitkFiducialRegistrationWidget.ui
  Qmitk/QmitkIGTLoggerWidgetControls.ui
  Qmitk/QmitkUpdateTimerWidgetControls.ui
  Qmitk/QmitkToolDistanceWidgetControls.ui
  Qmitk/QmitkToolTrackingStatusWidgetControls.ui
  Qmitk/QmitkTrackingSourcesCheckBoxPanelWidgetControls.ui
  Qmitk/QmitkIGTPlayerWidgetControls.ui
  Qmitk/QmitkIGTConnectionWidgetControls.ui
  Qmitk/QmitkToolSelectionWidgetControls.ui
  Qmitk/QmitkNavigationToolCreationWidget.ui
  Qmitk/QmitkNavigationDataSourceSelectionWidgetControls.ui
  Qmitk/QmitkInteractiveTransformationWidgetControls.ui
  Qmitk/QmitkNavigationToolStorageSelectionWidgetControls.ui

  Qmitk/QmitkNavigationDataPlayerControlWidget.ui
  Qmitk/QmitkNavigationDataSequentialPlayerControlWidget.ui

  Qmitk/QmitkNDIAuroraWidget.ui
  Qmitk/QmitkNDIPolarisWidget.ui
  Qmitk/QmitkMicronTrackerWidget.ui
  Qmitk/QmitkNPOptitrackWidget.ui
  Qmitk/QmitkVirtualTrackerWidget.ui
  Qmitk/QmitkOpenIGTLinkWidget.ui
)

set(MOC_H_FILES
  Qmitk/QmitkNavigationToolManagementWidget.h
  Qmitk/QmitkTrackingDeviceWidget.h
  Qmitk/QmitkTrackingDeviceConfigurationWidget.h
  Qmitk/QmitkNDIConfigurationWidget.h
  Qmitk/QmitkFiducialRegistrationWidget.h
  Qmitk/QmitkNDIToolDelegate.h
  Qmitk/QmitkIGTLoggerWidget.h
  Qmitk/QmitkUpdateTimerWidget.h
  Qmitk/QmitkToolDistanceWidget.h
  Qmitk/QmitkToolTrackingStatusWidget.h
  Qmitk/QmitkTrackingSourcesCheckBoxPanelWidget.h
  Qmitk/QmitkIGTPlayerWidget.h
  Qmitk/QmitkIGTConnectionWidget.h
  Qmitk/QmitkToolSelectionWidget.h
  Qmitk/QmitkNavigationToolCreationWidget.h
  Qmitk/QmitkNavigationDataSourceSelectionWidget.h
  Qmitk/QmitkInteractiveTransformationWidget.h
  Qmitk/QmitkNavigationToolStorageSelectionWidget.h

  Qmitk/QmitkNavigationDataPlayerControlWidget.h
  Qmitk/QmitkNavigationDataSequentialPlayerControlWidget.h

  Qmitk/QmitkNDIAuroraWidget.h
  Qmitk/QmitkNDIPolarisWidget.h
  Qmitk/QmitkMicronTrackerWidget.h
  Qmitk/QmitkNPOptitrackWidget.h
  Qmitk/QmitkVirtualTrackerWidget.h
  Qmitk/QmitkOpenIGTLinkWidget.h

  Qmitk/QmitkTrackingDeviceConfigurationWidgetScanPortsWorker.h
  Qmitk/QmitkTrackingDeviceConfigurationWidgetConnectionWorker.h
  Qmitk/QmitkNDIAbstractDeviceWidget.h
  Qmitk/QmitkAbstractTrackingDeviceWidget.h
)

if(MITK_USE_POLHEMUS_TRACKER)
  set(CPP_FILES ${CPP_FILES} Qmitk/QmitkPolhemusTrackerWidget.cpp)
  set(UI_FILES ${UI_FILES} Qmitk/QmitkPolhemusTrackerWidget.ui)
  set(MOC_H_FILES ${MOC_H_FILES} Qmitk/QmitkPolhemusTrackerWidget.h)
endif(MITK_USE_POLHEMUS_TRACKER)

set(QRC_FILES
  resources/IGTUI.qrc
)
