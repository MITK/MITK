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

#ifndef QmitkNDIPolarisWidget_H
#define QmitkNDIPolarisWidget_H

#include "MitkIGTUIExports.h"
#include "ui_QmitkNDIPolarisWidget.h"

#include "QmitkNDIAbstractDeviceWidget.h"

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
class MITKIGTUI_EXPORT QmitkNDIPolarisWidget : public QmitkNDIAbstractDeviceWidget
{
  Q_OBJECT // this is needed for all Qt objects that should have a MOC object (everything that derives from QObject)

public:
  static const std::string VIEW_ID;

  QmitkNDIPolarisWidget(QWidget* parent = 0, Qt::WindowFlags f = 0);
  ~QmitkNDIPolarisWidget();

signals:

  protected slots :

protected:

  /// \brief Creation of the connections
  virtual void CreateConnections();

  virtual void CreateQtPartControl(QWidget *parent);

  /** @return Returns the frame rate set in the m_frameRatePolaris ComboBox
  */
  mitk::IlluminationActivationRate GetPolarisFrameRate();

  Ui::QmitkNDIPolarisWidget* m_Controls;

  virtual void SetPortValueToGUI(int portValue);
  virtual void SetPortTypeToGUI(int portType);

public:
  virtual void ResetOutput();
  virtual void AddOutput(std::string s);
  virtual mitk::TrackingDevice::Pointer ConstructTrackingDevice();

  virtual void StoreUISettings();
  virtual void LoadUISettings();
};
#endif
