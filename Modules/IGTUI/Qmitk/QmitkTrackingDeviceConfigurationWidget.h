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
#include "mitkTrackingDevice.h"
#include <mitkIPersistenceService.h>
#include "mitkTrackingDeviceTypeCollection.h"
#include "mitkTrackingDeviceWidgetCollection.h"

//standard tracking devices, which always should be avaiable
#include "QmitkNDIAuroraWidget.h"
#include "QmitkNDIPolarisWidget.h"
#include "QmitkMicronTrackerWidget.h"
#include "QmitkNPOptitrackWidget.h"
#include "QmitkVirtualTrackerWidget.h"
#include "QmitkOpenIGTLinkWidget.h"

//itk headers

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

  /* @brief Resets the UI to allow the user for configurating a new tracking device.
   */
  void Reset();

  /** @brief External call to disable this widget when configuration is finished. This is also called by the "finished" button,
    *        but if you disable the advanced user control you might want to call this when the configuration is finished.
    *        If you want to configure a new device call the Reset() funktion later.
    */
  void ConfigurationFinished();

  /** @return Returns true if the tracking device is completely configured (you can get it by calling GetTrackingDevice() in this case).
   *          Returns false if configuration is not finished.
   */
  bool GetTrackingDeviceConfigured();

signals:

  /* @brief This signal is sent if the user has finished the configuration of the tracking device.
   *        The device is now availiable if the method GetTrackingDevice() is called. The tracking
   *        device you'll get is completly configurated but no tools are added yet.
   */
  void TrackingDeviceConfigurationFinished();

  /* @brief This signal is sent if the UI was reseted and the user is required to configurate
   *        a new tracking device.
   */
  void TrackingDeviceConfigurationReseted();

  /* @brief This signal is sent if the tracking device was changed. */
  void TrackingDeviceSelectionChanged();

protected:

  /// \brief Creation of the connections
  virtual void CreateConnections();

  virtual void CreateQtPartControl(QWidget *parent);

  Ui::QmitkTrackingDeviceConfigurationWidgetControls* m_Controls;

  mitk::TrackingDevice::Pointer m_TrackingDevice;

  bool m_TrackingDeviceConfigurated;

  // key is port name (e.g. "COM1", "/dev/ttyS0"), value will be filled with the type of tracking device at this port
  typedef QMap<QString, mitk::TrackingDeviceType> PortDeviceMap;

  //######################### internal help methods #######################################
  void ResetOutput();
  void AddOutput(std::string s);
  mitk::TrackingDevice::Pointer ConstructTrackingDevice();

  void StoreUISettings();
  void LoadUISettings();

  QmitkAbstractTrackingDeviceWidget* GetCurrentWidget();

  protected slots:
  /* @brief This method is called when the user changes the selection of the trackingdevice (m_trackingDeviceChooser).
            It then sets the correct widget for the selected tracking device.*/
  void TrackingDeviceChanged();

  /* @brief This method is called when the user clicks on "Refresh Selection" (m_RefreshTrackingDeviceCollection).
  It then sets the correct widget for the selected tracking device.*/
  void RefreshTrackingDeviceCollection();

  /* @brief This method is called when the user presses the button "finished". A new tracking device will be created in this case and will then
   *        then be availiable by calling GetTrackingDevice(). Also a signal TrackingDeviceConfigurationFinished() will be emitted. After this the
   *        UI will be disablet until the widget is reseted to configure a new tracking device.
   */
  void Finished();

  /* @brief This method is called when the user presses the button "reset". He can configure a new tracking device then. The signal
   *        TrackingDeviceConfigurationReseted() will be emitted if this method is called. The method GetTrackingDevice() will return
   *        NULL until a new tracking device is configured.
   */
  void ResetByUser();

private:
  PERSISTENCE_GET_SERVICE_METHOD_MACRO

    mitk::TrackingDeviceTypeCollection* m_DeviceTypeCollection;

  QmitkNDIAuroraWidget* m_auroraWidget;
  QmitkNDIPolarisWidget* m_polarisWidget;
  QmitkMicronTrackerWidget* m_microntrackerWidget;
  QmitkNPOptitrackWidget* m_optitrackWidget;
  QmitkVirtualTrackerWidget* m_virtualtrackerWidget;
  QmitkOpenIGTLinkWidget* m_openIGTLinkWidget;

  mitk::TrackingDeviceWidgetCollection m_DeviceWidgetCollection;
};

#endif
