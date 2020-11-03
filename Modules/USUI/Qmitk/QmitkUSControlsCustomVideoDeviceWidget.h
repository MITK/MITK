/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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

  /**
  * \brief Called when user changes the scanning depth of the ultrasound
  */
  void OnDepthChanged();

  /**
  * \brief Called when user chagnes the identifier for the probe of the ultrasoud machine
  */
  void OnProbeChanged();

  /**
  * \brief Get all depths for the given probe and fill them into a combobox
  */
  void SetDepthsForProbe(std::string probename);
public:

  QmitkUSControlsCustomVideoDeviceWidget();
  ~QmitkUSControlsCustomVideoDeviceWidget() override;

  /**
    * Getter for the device class of mitk:USVideoDevice.
    */
  std::string GetDeviceClass() const override;

  /**
    * Creates new QmitkUSAbstractCustomWidget with the same mitk::USVideoDevice
    * and the same mitk::USVideoDeviceCustomControls which were set on the
    * original object.
    *
    * This method is just for being calles by the factory. Use
    * QmitkUSAbstractCustomWidget::CloneForQt() instead, if you want a clone of
    * an object.
    */
  QmitkUSAbstractCustomWidget* Clone(QWidget* parent = nullptr) const override;

  /**
    * Gets control interface from the device which was currently set. Control
    * elements are according to current crop area of the device. If custom
    * control interface is null, the control elements stay disabled.
    */
  void OnDeviceSet() override;

  void Initialize() override;

protected:
  void BlockSignalAndSetValue(QSpinBox* target, int value);

  mitk::USImageVideoSource::USImageCropping m_Cropping;

private:
  /**
  * Constructs widget object. All gui control elements will be disabled until
  * QmitkUSAbstractCustomWidget::SetDevice() was called.
  */
  QmitkUSControlsCustomVideoDeviceWidget(QWidget *parent);
  Ui::QmitkUSControlsCustomVideoDeviceWidget*         ui;

  mitk::USVideoDeviceCustomControls::Pointer  m_ControlInterface;
};

#endif // QmitkUSControlsCustomVideoDeviceWidget_H
