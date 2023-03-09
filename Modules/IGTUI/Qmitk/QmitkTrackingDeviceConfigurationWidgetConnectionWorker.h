/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef QmitkTrackingDeviceConfigurationWidgetConnectionWorker_h
#define QmitkTrackingDeviceConfigurationWidgetConnectionWorker_h

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
  ~QmitkTrackingDeviceConfigurationWidgetConnectionWorker() override{};
  void SetTrackingDevice(mitk::TrackingDevice::Pointer t);

public slots:
  void TestConnectionThreadFunc();

signals:
  void ConnectionTested(bool connected, QString output);

protected:
  mitk::TrackingDevice::Pointer m_TrackingDevice;
};
#endif
