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

#ifndef QmitkPolhemusTrackerWidget_H
#define QmitkPolhemusTrackerWidget_H

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

private:

  /// \brief Creation of the connections
  void CreateConnections();

  void CreateQtPartControl(QWidget *parent);

  void SetAdvancedSettingsVisible(bool _enable);

  int GetSelectedToolIndex();

protected:
  virtual QmitkPolhemusTrackerWidget* Clone(QWidget* parent) const;

  Ui::QmitkPolhemusTrackerWidget* m_Controls;

  mitk::PolhemusTrackingDevice::Pointer m_TrackingDevice;

public:
  virtual mitk::TrackingDevice::Pointer ConstructTrackingDevice();
  /**
  * \brief This function is called, when in the TrackingToolboxView "Connect" was clicked and the device is successful connected.
  * Can e.g. be used to activate options of a tracking device only when it is connected.
  */
  virtual void OnConnected();
  /**
  * \brief This function is called, when in the TrackingToolboxView "Disconnect" was clicked and the device is successful disconnected.
  * Can e.g. be used to activate/disactivate options of a tracking device.
  */
  virtual void OnDisconnected();
};
#endif
