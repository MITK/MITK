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

#include <QWidget>
#include "MitkIGTUIExports.h"
#include "ui_QmitkNPOptitrackWidget.h"

#include "QmitkAbstractTrackingDeviceWidget.h"

//itk headers

/** Documentation:
 *   \brief Simple and fast access to a pre-configured TrackingDeviceSource.
 *
 *   This widget creates a fully configured, connected and started TrackingDeviceSource.
 *   Clicking "Connect" requires to specify a NavigationToolStorage that holds all tools to be used
 *   in the application. Corresponding surfaces are added to the DataStorage that has to be set for
 *   the widget.
 *
 *   Inputs: DataStorage
 *   Outputs: TrackingDeviceSource, NavigationToolStorage
 *   Signals: TrackingDeviceConnected, TrackingDeviceDisconnected
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

signals:

  protected slots :

    /* @brief Opens a file dialog. The users sets the calibration file which location is then stored in the member m_OptitrackCalibrationFile.*/
    void SetOptitrackCalibrationFileClicked();

protected:

  /// \brief Creation of the connections
  virtual void CreateConnections();

  virtual void CreateQtPartControl(QWidget *parent);

  std::string m_OptitrackCalibrationFile;

  Ui::QmitkNPOptitrackWidget* m_Controls;

public:
  virtual void ResetOutput();
  virtual void AddOutput(std::string s);
  virtual mitk::TrackingDevice::Pointer ConstructTrackingDevice();

  virtual void StoreUISettings();
  virtual void LoadUISettings();
  virtual bool IsDeviceInstalled();
};
#endif
