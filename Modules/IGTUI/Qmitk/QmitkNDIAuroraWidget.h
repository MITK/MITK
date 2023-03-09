/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkNDIAuroraWidget_h
#define QmitkNDIAuroraWidget_h

#include "ui_QmitkNDIAuroraWidget.h"
#include "QmitkNDIAbstractDeviceWidget.h"


/** Documentation:
*   \brief Implementation of a configuration widget for NDI Aurora Devices.
*
*   \ingroup IGTUI
*/
class MITKIGTUI_EXPORT QmitkNDIAuroraWidget : public QmitkNDIAbstractDeviceWidget
{
  Q_OBJECT // this is needed for all Qt objects that should have a MOC object (everything that derives from QObject)

public:
  static const std::string VIEW_ID;

  QmitkNDIAuroraWidget(QWidget* parent = nullptr, Qt::WindowFlags f = nullptr);
  ~QmitkNDIAuroraWidget() override;

  void Initialize() override;

private:
  /// \brief Creation of the connections
  void CreateConnections();

  void CreateQtPartControl(QWidget *parent);
protected:
  void ResetOutput() override;
  void AddOutput(std::string s) override;
  mitk::TrackingDevice::Pointer GetTrackingDevice() override;

  void StoreUISettings() override;
  void LoadUISettings() override;

  void SetPortValueToGUI(int portValue) override;
  void SetPortTypeToGUI(int portType) override;

  QmitkNDIAuroraWidget* Clone(QWidget* parent) const override;

  Ui::QmitkNDIAuroraWidget* m_Controls;
};

#endif
