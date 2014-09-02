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

#include "mitkUSVideoDevice.h"

#include <QSpinBox>

namespace Ui {
class QmitkUSControlsCustomVideoDeviceWidget;
}

/** \brief Widget for custom controls of mitk::USVideoDevice.
  * This class handles the itk::USVideoDeviceCustomControls of video device
  * objects.
  */
class QmitkUSControlsCustomVideoDeviceWidget : public QmitkUSAbstractCustomWidget
{
    Q_OBJECT

private slots:
  /**
    * \brief Called when user changes one of the crop area control elements.
    */
  void OnCropAreaChanged();

public:
  /**
    * Constructs widget object. All gui control elements will be disabled until
    * QmitkUSAbstractCustomWidget::SetDevice() was called.
    */
  explicit QmitkUSControlsCustomVideoDeviceWidget(QWidget *parent = 0);
  ~QmitkUSControlsCustomVideoDeviceWidget();

  /**
    * Getter for the device class of mitk:USVideoDevice.
    */
  virtual std::string GetDeviceClass() const;

  /**
    * Creates new QmitkUSAbstractCustomWidget with the same mitk::USVideoDevice
    * and the same mitk::USVideoDeviceCustomControls which were set on the
    * original object.
    *
    * This method is just for being calles by the factory. Use
    * QmitkUSAbstractCustomWidget::CloneForQt() instead, if you want a clone of
    * an object.
    */
  virtual QmitkUSAbstractCustomWidget* Clone(QWidget* parent = 0) const;

  /**
    * Gets control interface from the device which was currently set. Control
    * elements are according to current crop area of the device. If custom
    * control interface is null, the control elements stay disabled.
    */
  virtual void OnDeviceSet();

  virtual void Initialize();

protected:
  void BlockSignalAndSetValue(QSpinBox* target, int value);

  mitk::USImageVideoSource::USImageCropping m_Cropping;

private:
  Ui::QmitkUSControlsCustomVideoDeviceWidget*         ui;

  mitk::USVideoDeviceCustomControls::Pointer  m_ControlInterface;
};

#endif // QmitkUSControlsCustomVideoDeviceWidget_H