/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkOpenIGTLinkWidget_h
#define QmitkOpenIGTLinkWidget_h

#include "ui_QmitkOpenIGTLinkWidget.h"
#include "QmitkAbstractTrackingDeviceWidget.h"


/** Documentation:
*   \brief Implementation of a configuration widget to use an Open IGT Link connection to track any device.
*
*   \ingroup IGTUI
*/
class MITKIGTUI_EXPORT QmitkOpenIGTLinkWidget : public QmitkAbstractTrackingDeviceWidget
{
  Q_OBJECT // this is needed for all Qt objects that should have a MOC object (everything that derives from QObject)

public:
  static const std::string VIEW_ID;

  QmitkOpenIGTLinkWidget(QWidget* parent = nullptr, Qt::WindowFlags f = nullptr);
  ~QmitkOpenIGTLinkWidget() override;

  void Initialize() override;

signals:

  protected slots :

private:
  void CreateQtPartControl(QWidget *parent);
protected:
  QmitkOpenIGTLinkWidget* Clone(QWidget* parent) const override;

  Ui::QmitkOpenIGTLinkWidget* m_Controls;
public:
  mitk::TrackingDevice::Pointer GetTrackingDevice() override;
};
#endif
