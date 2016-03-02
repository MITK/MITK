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

#ifndef QmitkMicronTrackerWidget_H
#define QmitkMicronTrackerWidget_H

#include "ui_QmitkMicronTrackerWidget.h"

#include "QmitkAbstractTrackingDeviceWidget.h"


/** Documentation:
 *   \brief Implementation of a configuration widget for Micron Tracking Devices.
 *
 *   \ingroup IGTUI
 */
class MITKIGTUI_EXPORT QmitkMicronTrackerWidget : public QmitkAbstractTrackingDeviceWidget
{
  Q_OBJECT // this is needed for all Qt objects that should have a MOC object (everything that derives from QObject)

public:
  static const std::string VIEW_ID;

  QmitkMicronTrackerWidget(QWidget* parent = 0, Qt::WindowFlags f = 0);
  ~QmitkMicronTrackerWidget();

  virtual void Initialize();

signals:

  protected slots :
    /* @brief Opens a file dialog. The users sets the calibration file which location is then stored in the member m_MTCalibrationFile.*/
    void SetMTCalibrationFileClicked();

private:

  /// \brief Creation of the connections
  void CreateConnections();

  void CreateQtPartControl(QWidget *parent);

protected:
  virtual QmitkMicronTrackerWidget* Clone(QWidget* parent) const;

  std::string m_MTCalibrationFile;

  Ui::QmitkMicronTrackerWidget* m_Controls;

public:
  virtual void ResetOutput();
  virtual void AddOutput(std::string s);
  virtual mitk::TrackingDevice::Pointer ConstructTrackingDevice();

  virtual void StoreUISettings();
  virtual void LoadUISettings();
  virtual bool IsDeviceInstalled();
};
#endif
