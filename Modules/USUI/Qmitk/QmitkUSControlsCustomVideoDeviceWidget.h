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

#ifndef QmitkUSControlsCustomVideoDeviceWidget_H
#define QmitkUSControlsCustomVideoDeviceWidget_H

#include "QmitkUSAbstractCustomWidget.h"

#include "mitkUSVideoDeviceCustomControls.h"

namespace Ui {
class QmitkUSControlsCustomVideoDeviceWidget;
}

class QmitkUSControlsCustomVideoDeviceWidget : public QmitkUSAbstractCustomWidget
{
    Q_OBJECT

private slots:
  void OnCropAreaChanged();

public:
  explicit QmitkUSControlsCustomVideoDeviceWidget(mitk::USVideoDeviceCustomControls::Pointer controlInterface, QWidget *parent = 0);
  explicit QmitkUSControlsCustomVideoDeviceWidget(QWidget *parent = 0);
  ~QmitkUSControlsCustomVideoDeviceWidget();

  virtual std::string GetDeviceClass() const;
  virtual QmitkUSAbstractCustomWidget* Clone(QWidget* parent = 0) const;

  virtual void OnDeviceSet();

private:
  Ui::QmitkUSControlsCustomVideoDeviceWidget*         ui;

  mitk::USVideoDeviceCustomControls::Pointer  m_ControlInterface;
};

#endif // QmitkUSControlsCustomVideoDeviceWidget_H