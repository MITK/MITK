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


#ifndef QmitkTrackingDeviceConfigurationWidgetConnectionWorker_H
#define QmitkTrackingDeviceConfigurationWidgetConnectionWorker_H

#include <QWidget>
#include "mitkTrackingDevice.h"

#include "MitkIGTUIExports.h"

// ###################################################################################################
//############ PRIVATE WORKER CLASSES FOR THREADS ###################################################
//###################################################################################################


/**
* Worker thread class for test connection.
*/
class MITKIGTUI_EXPORT QmitkTrackingDeviceConfigurationWidgetConnectionWorker : public QObject
{
  Q_OBJECT;
public:
  QmitkTrackingDeviceConfigurationWidgetConnectionWorker(){};
  ~QmitkTrackingDeviceConfigurationWidgetConnectionWorker(){};
  void SetTrackingDevice(mitk::TrackingDevice::Pointer t);

public slots:
  void TestConnectionThreadFunc();

signals:
  void ConnectionTested(bool connected, QString output);

protected:
  mitk::TrackingDevice::Pointer m_TrackingDevice;
};
#endif
