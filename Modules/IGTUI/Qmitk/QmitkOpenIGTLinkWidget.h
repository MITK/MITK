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

#ifndef QmitkOpenIGTLinkWidget_H
#define QmitkOpenIGTLinkWidget_H

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

  QmitkOpenIGTLinkWidget(QWidget* parent = 0, Qt::WindowFlags f = 0);
  ~QmitkOpenIGTLinkWidget();

  virtual void Initialize();

signals:

  protected slots :

private:
  void CreateQtPartControl(QWidget *parent);
protected:
  virtual QmitkOpenIGTLinkWidget* Clone(QWidget* parent) const;

  Ui::QmitkOpenIGTLinkWidget* m_Controls;
public:
  virtual mitk::TrackingDevice::Pointer ConstructTrackingDevice();
};
#endif
