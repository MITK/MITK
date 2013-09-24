
set(CPP_FILES
  mitkUSUIActivator.cpp
  Qmitk/QmitkUSDeviceManagerWidget.cpp
  Qmitk/QmitkUSNewVideoDeviceWidget.cpp
  Qmitk/QmitkUSControlsBModeWidget.cpp
  Qmitk/QmitkUSControlsDopplerWidget.cpp
  Qmitk/QmitkUSControlsProbesWidget.cpp
  Qmitk/QmitkUSControlsCustomVideoDeviceWidget.cpp
  Qmitk/QmitkUSAbstractCustomWidget.cpp
)
set(UI_FILES
  Qmitk/QmitkUSDeviceManagerWidgetControls.ui
  Qmitk/QmitkUSNewVideoDeviceWidgetControls.ui
  Qmitk/QmitkUSControlsBModeWidget.ui
  Qmitk/QmitkUSControlsDopplerWidget.ui
  Qmitk/QmitkUSControlsProbesWidget.ui
  Qmitk/QmitkUSControlsCustomVideoDeviceWidget.ui
)

set(MOC_H_FILES
  Qmitk/QmitkUSDeviceManagerWidget.h
  Qmitk/QmitkUSNewVideoDeviceWidget.h
  Qmitk/QmitkUSControlsBModeWidget.h
  Qmitk/QmitkUSControlsDopplerWidget.h
  Qmitk/QmitkUSControlsProbesWidget.h
  Qmitk/QmitkUSControlsCustomVideoDeviceWidget.h
  Qmitk/QmitkUSAbstractCustomWidget.h
)

# uncomment the following line if you want to use Qt resources
set(QRC_FILES
#  resources/QmitkToFUtilWidget.qrc
)
