/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkNPOptitrackWidget_h
#define QmitkNPOptitrackWidget_h

#include "ui_QmitkNPOptitrackWidget.h"
#include "QmitkAbstractTrackingDeviceWidget.h"


/** Documentation:
*   \brief Implementation of a configuration widget for NP Optitrack Tracking Devices.
*
*   \ingroup IGTUI
*/
class MITKIGTUI_EXPORT QmitkNPOptitrackWidget : public QmitkAbstractTrackingDeviceWidget
{
  Q_OBJECT // this is needed for all Qt objects that should have a MOC object (everything that derives from QObject)

public:
  static const std::string VIEW_ID;

  QmitkNPOptitrackWidget(QWidget* parent = nullptr, Qt::WindowFlags f = nullptr);
  ~QmitkNPOptitrackWidget() override;

  void Initialize() override;

signals:

  protected slots :

    /* @brief Opens a file dialog. The users sets the calibration file which location is then stored in the member m_OptitrackCalibrationFile.*/
    void SetOptitrackCalibrationFileClicked();

private:

  /// \brief Creation of the connections
  void CreateConnections();

  void CreateQtPartControl(QWidget *parent);
protected:
  QmitkNPOptitrackWidget* Clone(QWidget* parent) const override;

  std::string m_OptitrackCalibrationFile;

  Ui::QmitkNPOptitrackWidget* m_Controls;

public:
  void ResetOutput() override;
  void AddOutput(std::string s) override;
  mitk::TrackingDevice::Pointer GetTrackingDevice() override;

  bool IsDeviceInstalled() override;
};
#endif
