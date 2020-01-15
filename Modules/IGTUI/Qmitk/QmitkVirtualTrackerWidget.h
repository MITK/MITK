/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkVirtualTrackerWidget_H
#define QmitkVirtualTrackerWidget_H

#include "ui_QmitkVirtualTrackerWidget.h"
#include "QmitkAbstractTrackingDeviceWidget.h"


/** Documentation:
*   \brief Implementation of a configuration widget for a Vitrual Tracking Device.
*
*   \ingroup IGTUI
*/
class MITKIGTUI_EXPORT QmitkVirtualTrackerWidget : public QmitkAbstractTrackingDeviceWidget
{
  Q_OBJECT // this is needed for all Qt objects that should have a MOC object (everything that derives from QObject)

public:
  static const std::string VIEW_ID;

  QmitkVirtualTrackerWidget(QWidget* parent = nullptr, Qt::WindowFlags f = nullptr);
  ~QmitkVirtualTrackerWidget() override;

  void Initialize() override;

signals:

  protected slots :
    /* @brief Enables or disables the Gaussian Noise for the VirtualTrackingDevice dependent on the State of the according Checkbox */
    void EnableGaussianNoise();

private:

  /// \brief Creation of the connections
  void CreateConnections();

  void CreateQtPartControl(QWidget *parent);
protected:
  QmitkVirtualTrackerWidget* Clone(QWidget* parent) const override;

  Ui::QmitkVirtualTrackerWidget* m_Controls;

public:
  mitk::TrackingDevice::Pointer GetTrackingDevice() override;
};
#endif
