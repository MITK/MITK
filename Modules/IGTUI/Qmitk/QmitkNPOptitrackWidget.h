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

#ifndef QmitkNPOptitrackWidget_H
#define QmitkNPOptitrackWidget_H

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

  QmitkNPOptitrackWidget(QWidget* parent = 0, Qt::WindowFlags f = 0);
  ~QmitkNPOptitrackWidget();

  virtual void Initialize();

signals:

  protected slots :

    /* @brief Opens a file dialog. The users sets the calibration file which location is then stored in the member m_OptitrackCalibrationFile.*/
    void SetOptitrackCalibrationFileClicked();

private:

  /// \brief Creation of the connections
  void CreateConnections();

  void CreateQtPartControl(QWidget *parent);
protected:
  virtual QmitkNPOptitrackWidget* Clone(QWidget* parent) const;

  std::string m_OptitrackCalibrationFile;

  Ui::QmitkNPOptitrackWidget* m_Controls;

public:
  virtual void ResetOutput();
  virtual void AddOutput(std::string s);
  virtual mitk::TrackingDevice::Pointer ConstructTrackingDevice();

  virtual bool IsDeviceInstalled();
};
#endif
