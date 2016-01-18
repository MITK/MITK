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

#ifndef QmitkTrackingDeviceConfigurationWidgetScanPortsWorker_H
#define QmitkTrackingDeviceConfigurationWidgetScanPortsWorker_H

#include <QWidget>
#include "mitkTrackingDevice.h"

#include "MitkIGTUIExports.h"

/**
* Worker thread class for scan ports.
*/
class MITKIGTUI_EXPORT QmitkTrackingDeviceConfigurationWidgetScanPortsWorker : public QObject
{
  Q_OBJECT;

public:
  QmitkTrackingDeviceConfigurationWidgetScanPortsWorker(){};
  ~QmitkTrackingDeviceConfigurationWidgetScanPortsWorker(){};

  public slots:
  void ScanPortsThreadFunc();

signals:

  /**
  * @param Port Returns the port, returns -1 if no device was found.
  * @param PortType Returns the port type (0=usb,1=tty), returns -1 if the port type is not specified, e.g, in case of Windows.
  */
  void PortsScanned(int Port, QString result, int PortType);

protected:

  /** @brief   Scans the given port for a NDI tracking device.
  * @return  Returns the type of the device if one was found. Returns TrackingSystemInvalid if none was found.
  */
  mitk::TrackingDeviceType ScanPort(QString port);
};
#endif
