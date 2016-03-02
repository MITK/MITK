/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

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

  QmitkTrackingDeviceConfigurationWidget(QWidget* parent = 0, Qt::WindowFlags f = 0);
  ~QmitkTrackingDeviceConfigurationWidget();

  /* @return Returns the current configurated tracking device. If the user didn't finished the
   *         configuration process or if there is an error during configuration NULL is returned.
   */
  mitk::TrackingDevice::Pointer GetTrackingDevice();

signals:

  /* @brief This signal is sent if the tracking device was changed. */
  void TrackingDeviceSelectionChanged();

protected:

  /// \brief Creation of the connections
  virtual void CreateConnections();

  virtual void CreateQtPartControl(QWidget *parent);

  Ui::QmitkTrackingDeviceConfigurationWidgetControls* m_Controls;

  mitk::TrackingDevice::Pointer m_TrackingDevice;

  // key is port name (e.g. "COM1", "/dev/ttyS0"), value will be filled with the type of tracking device at this port
  typedef QMap<QString, mitk::TrackingDeviceType> PortDeviceMap;

  //######################### internal help methods #######################################
  void ResetOutput();
  void AddOutput(std::string s);
  mitk::TrackingDevice::Pointer ConstructTrackingDevice();

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
