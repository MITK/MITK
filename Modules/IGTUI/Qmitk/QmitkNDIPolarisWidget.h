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

  QmitkNDIPolarisWidget(QWidget* parent = nullptr, Qt::WindowFlags f = nullptr);
  ~QmitkNDIPolarisWidget() override;

  void Initialize() override;

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

  void SetPortValueToGUI(int portValue) override;
  void SetPortTypeToGUI(int portType) override;

  QmitkNDIPolarisWidget* Clone(QWidget* parent) const override;

public:
  void ResetOutput() override;
  void AddOutput(std::string s) override;
  mitk::TrackingDevice::Pointer GetTrackingDevice() override;

  void StoreUISettings() override;
  void LoadUISettings() override;
};
#endif
