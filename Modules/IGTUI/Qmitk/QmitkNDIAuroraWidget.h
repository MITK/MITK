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

#ifndef QmitkNDIAuroraWidget_H
#define QmitkNDIAuroraWidget_H

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

  QmitkNDIAuroraWidget(QWidget* parent = 0, Qt::WindowFlags f = 0);
  ~QmitkNDIAuroraWidget();

  virtual void Initialize();

private:
  /// \brief Creation of the connections
  void CreateConnections();

  void CreateQtPartControl(QWidget *parent);
protected:
  virtual void ResetOutput();
  virtual void AddOutput(std::string s);
  virtual mitk::TrackingDevice::Pointer ConstructTrackingDevice();

  virtual void StoreUISettings();
  virtual void LoadUISettings();

  virtual void SetPortValueToGUI(int portValue);
  virtual void SetPortTypeToGUI(int portType);

  virtual QmitkNDIAuroraWidget* Clone(QWidget* parent) const;

  Ui::QmitkNDIAuroraWidget* m_Controls;
};

#endif
