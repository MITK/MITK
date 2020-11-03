/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkNDIAbstractDeviceWidget_H
#define QmitkNDIAbstractDeviceWidget_H

#include "MitkIGTUIExports.h"
#include "QmitkAbstractTrackingDeviceWidget.h"
#include "QmitkTrackingDeviceConfigurationWidgetScanPortsWorker.h"

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

  QmitkNDIAbstractDeviceWidget(QWidget* parent = nullptr, Qt::WindowFlags f = nullptr);
  ~QmitkNDIAbstractDeviceWidget() override;

  void Initialize() override = 0;

  void AddOutput(std::string s) override = 0;

signals:
  void PortsScanned(int Port, QString result, int PortType);

  protected slots:

  /* @brief Scans the serial ports automatically for a connected tracking device. If the method finds a device
  *        it selects the right type and sets the corresponding port in the widget.
  */
  void AutoScanPorts();

  /** This slot is called when the port scanning is finished. */
  void AutoScanPortsFinished(int Port, QString result, int PortType);

private:
  /// \brief Creation of the connections
  void CreateConnections();
protected:
  void InitializeNDIWidget();

  QmitkTrackingDeviceConfigurationWidgetScanPortsWorker* m_ScanPortsWorker;
  QThread* m_ScanPortsWorkerThread;

  virtual void SetPortValueToGUI(int portValue) = 0;
  virtual void SetPortTypeToGUI(int portType) = 0;
};

#endif
