/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QMITKTRACKINGDEVICECONFIGURATIONWIDGET_H
#define QMITKTRACKINGDEVICECONFIGURATIONWIDGET_H

#include <QWidget>
#include <QThread>
#include "MitkIGTUIExports.h"
#include "ui_QmitkTrackingDeviceConfigurationWidgetControls.h"
#include "mitkTrackingDeviceTypeCollection.h"
#include "mitkTrackingDeviceWidgetCollection.h"


/** Documentation:
 *   \brief An object of this class offers an UI to configurate
 *          a tracking device. If the user finished the configuration process and
 *          a fully configurated tracking device is availiabe the object emits a
 *          signal "TrackingDeviceConfigurationFinished()". You can then get the
 *          tracking device by calling the method GetTrackingDevice().
 *
 *          Once the tracking device is configurated there are two ways to reset
 *          the UI to allow the user for configuring a new device. The method Reset()
 *          can be called and there is also a button "reset" which can be pressed by
 *          the user. In both cases a signal "TrackingDeviceConfigurationReseted()"
 *          is emitted and you may wait for a new configurated tracking device.
 *
 *
 *   \ingroup IGTUI
 */
class MITKIGTUI_EXPORT QmitkTrackingDeviceConfigurationWidget : public QWidget
{
  Q_OBJECT

public:
  static const std::string VIEW_ID;

  QmitkTrackingDeviceConfigurationWidget(QWidget* parent = nullptr, Qt::WindowFlags f = nullptr);
  ~QmitkTrackingDeviceConfigurationWidget() override;

  /* @return Returns the current configurated tracking device. If the user didn't finished the
   *         configuration process or if there is an error during configuration nullptr is returned.
   */
  mitk::TrackingDevice::Pointer GetTrackingDevice();

  /**
  * \brief This function is called, when anything in the ToolStorage changed, e.g. AddTool or EditTool.
  * ServiceListener is connected in the QmitkMITKIGTTrackingToolboxView.
  */
  void OnToolStorageChanged();

signals:

  /* @brief This signal is sent if the tracking device was changed. */
  void TrackingDeviceSelectionChanged();

public slots:
/**
* \brief This function is called, when in the TrackingToolboxView "Connect" was clicked and the device is successful connected.
* Can e.g. be used to activate options of a tracking device only when it is connected.
*/
  void OnConnected(bool _success);
  /**
  * \brief This function is called, when in the TrackingToolboxView "Disconnect" was clicked and the device is successful disconnected.
  * Can e.g. be used to activate/disactivate options of a tracking device.
  */
  void OnDisconnected(bool _success);

  /**
  * \brief This function is called, when in the TrackingToolboxView "Start Tracking" was clicked and the device successfully started tracking.
  * Can e.g. be used to activate options of a tracking device only when tracking is started.
  */
  void OnStartTracking(bool _success);
  /**
  * \brief This function is called, when in the TrackingToolboxView "Stop Tracking" was clicked and the device successful stopped tracking.
  * Can e.g. be used to activate/disactivate options when device is not tracking.
  */
  void OnStopTracking(bool _success);

protected:

  /// \brief Creation of the connections
  virtual void CreateConnections();

  virtual void CreateQtPartControl(QWidget *parent);

  Ui::QmitkTrackingDeviceConfigurationWidgetControls* m_Controls;

  // key is port name (e.g. "COM1", "/dev/ttyS0"), value will be filled with the type of tracking device at this port
  typedef QMap<QString, mitk::TrackingDeviceType> PortDeviceMap;

  //######################### internal help methods #######################################
  void ResetOutput();
  void AddOutput(std::string s);

  void StoreUISettings();
  void LoadUISettings();

  /* @brief This method is called when the user clicks on "Refresh Selection" (m_RefreshTrackingDeviceCollection).
  It then sets the correct widget for the selected tracking device.*/
  void RefreshTrackingDeviceCollection();

  protected slots:
  /* @brief This method is called when the user changes the selection of the trackingdevice (m_trackingDeviceChooser).
            It then sets the correct widget for the selected tracking device.*/
  void TrackingDeviceChanged();

private:
  PERSISTENCE_GET_SERVICE_METHOD_MACRO

  std::string GetCurrentDeviceName(void) const;

  QmitkAbstractTrackingDeviceWidget* GetWidget(const std::string& deviceName) const;

  /**
   * @brief Mapping of device type identifier and index of the configuration widget in QStackedWidget.
   */
  std::map<std::string, int> m_DeviceToWidgetIndexMap;

};

#endif
