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

#ifndef QmitkNDIAbstractDeviceWidget_H
#define QmitkNDIAbstractDeviceWidget_H

#include <QWidget>
#include "MitkIGTUIExports.h"
#include "QmitkAbstractTrackingDeviceWidget.h"

#include "QmitkTrackingDeviceConfigurationWidgetScanPortsWorker.h"

//itk headers

/** Documentation:
*   \brief Abstract class of a configuration widget for NDI Devices.
*          For implementations see NDIAuroraWidget or NDIPolarisWidget.
*
*   \ingroup IGTUI
*/
class MITKIGTUI_EXPORT QmitkNDIAbstractDeviceWidget : public QmitkAbstractTrackingDeviceWidget
{
  Q_OBJECT // this is needed for all Qt objects that should have a MOC object (everything that derives from QObject)

public:
  static const std::string VIEW_ID;

  QmitkNDIAbstractDeviceWidget(QWidget* parent = 0, Qt::WindowFlags f = 0);
  ~QmitkNDIAbstractDeviceWidget();

  virtual void AddOutput(std::string s) = 0;

signals:
  void PortsScanned(int Port, QString result, int PortType);

  protected slots:

  /* @brief Scans the serial ports automatically for a connected tracking device. If the method finds a device
  *        it selects the right type and sets the corresponding port in the widget.
  */
  void AutoScanPorts();

  /** This slot is called when the port scanning is finished. */
  void AutoScanPortsFinished(int Port, QString result, int PortType);

protected:

  /// \brief Creation of the connections
  virtual void CreateConnections();

  virtual void CreateQtPartControl(QWidget *parent){};

  QThread* m_ScanPortsWorkerThread;
  QmitkTrackingDeviceConfigurationWidgetScanPortsWorker* m_ScanPortsWorker;

  virtual void SetPortValueToGUI(int portValue) = 0;
  virtual void SetPortTypeToGUI(int portType) = 0;

};

#endif
