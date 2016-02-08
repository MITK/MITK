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

#include "ui_QmitkNDIPolarisWidget.h"
#include "QmitkNDIAbstractDeviceWidget.h"


/** Documentation:
*   \brief Implementation of a configuration widget for NDI Polaris Devices.
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

  virtual void Initialize();

signals:

  protected slots :

private:
  /// \brief Creation of the connections
  void CreateConnections();

  void CreateQtPartControl(QWidget *parent);
protected:

  /** @return Returns the frame rate set in the m_frameRatePolaris ComboBox
  */
  mitk::IlluminationActivationRate GetPolarisFrameRate();

  Ui::QmitkNDIPolarisWidget* m_Controls;

  virtual void SetPortValueToGUI(int portValue);
  virtual void SetPortTypeToGUI(int portType);

  virtual QmitkNDIPolarisWidget* Clone(QWidget* parent) const;

public:
  virtual void ResetOutput();
  virtual void AddOutput(std::string s);
  virtual mitk::TrackingDevice::Pointer ConstructTrackingDevice();

  virtual void StoreUISettings();
  virtual void LoadUISettings();
};
#endif
