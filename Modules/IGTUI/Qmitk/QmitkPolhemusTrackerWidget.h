/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkPolhemusTrackerWidget_h
#define QmitkPolhemusTrackerWidget_h

#include "ui_QmitkPolhemusTrackerWidget.h"

#include "QmitkAbstractTrackingDeviceWidget.h"
#include <mitkPolhemusTrackingDevice.h>


/** Documentation:
 *   \brief Implementation of a configuration widget for Polhemus Tracking Devices.
 *
 *   \ingroup IGTUI
 */
class MITKIGTUI_EXPORT QmitkPolhemusTrackerWidget : public QmitkAbstractTrackingDeviceWidget
{
  Q_OBJECT // this is needed for all Qt objects that should have a MOC object (everything that derives from QObject)

public:
  static const std::string VIEW_ID;

  QmitkPolhemusTrackerWidget(QWidget* parent = 0, Qt::WindowFlags f = 0);
  ~QmitkPolhemusTrackerWidget();

  virtual void Initialize();

signals:

  protected slots :
    void on_m_hemisphereTracking_clicked();
    void on_m_ToggleHemisphere_clicked();
    void on_m_SetHemisphere_clicked();
    void on_m_GetHemisphere_clicked();
    void on_m_AdjustHemisphere_clicked();
    void on_m_AdvancedSettings_clicked();
    void on_m_ToggleToolTipCalibration_clicked();

private:

  /// \brief Creation of the connections
  void CreateConnections();

  void CreateQtPartControl(QWidget *parent);

  void SetAdvancedSettingsEnabled(bool _enable);

  int GetSelectedToolIndex();

protected:
  virtual QmitkPolhemusTrackerWidget* Clone(QWidget* parent) const;

  Ui::QmitkPolhemusTrackerWidget* m_Controls;

  mitk::PolhemusTrackingDevice::Pointer m_TrackingDevice;

public:
  virtual mitk::TrackingDevice::Pointer GetTrackingDevice();
  /**
  * \brief This function is called, when in the TrackingToolboxView "Connect" was clicked and the device is successful connected.
  * Can e.g. be used to activate options of a tracking device only when it is connected.
  */
  virtual void OnConnected( bool _success);
  /**
  * \brief This function is called, when in the TrackingToolboxView "Disconnect" was clicked and the device is successful disconnected.
  * Can e.g. be used to activate/disactivate options of a tracking device.
  */
  virtual void OnDisconnected(bool _success);
  /**
  * \brief This function is called, when in the TrackingToolboxView "Start Tracking" was clicked and the device successfully started tracking.
  * Can e.g. be used to activate options of a tracking device only when tracking is started.
  */
  virtual void OnStartTracking(bool _success);
  /**
  * \brief This function is called, when anything in the ToolStorage changed, e.g. AddTool or EditTool.
  * ServiceListener is connected in the QmitkMITKIGTTrackingToolboxView.
  */
  virtual void OnToolStorageChanged();

};
#endif
