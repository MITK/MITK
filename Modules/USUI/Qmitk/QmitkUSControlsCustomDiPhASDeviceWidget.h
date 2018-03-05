
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

#ifndef QmitkUSControlsCustomDiPhASDeviceWidget_H
#define QmitkUSControlsCustomDiPhASDeviceWidget_H

#include "QmitkUSAbstractCustomWidget.h"
#include "mitkUSDiPhASDeviceCustomControls.h"

#include "mitkUSDevice.h"

#include <QSpinBox>

namespace Ui {
class QmitkUSControlsCustomDiPhASDeviceWidget;
}

/** \brief Widget for custom controls of mitk::USDiPhASDevice.
  * This class handles the itk::USDiPhASDeviceCustomControls of video device
  * objects.
  */
class QmitkUSControlsCustomDiPhASDeviceWidget : public QmitkUSAbstractCustomWidget
{
    Q_OBJECT

private slots:
  virtual void OnCompensateEnergyChanged();
  virtual void OnUseBModeFilterChanged();
  virtual void OnVerticalSpacingChanged();
  virtual void OnRecordChanged();
  virtual void OnScatteringCoefficientChanged();
  virtual void OnCompensateScatteringChanged();
  virtual void OnChangedSavingSettings();

  //Transmit
  virtual void OnTransmitPhaseLengthChanged();
  virtual void OnExcitationFrequencyChanged();
  virtual void OnTransmitEventsChanged();
  virtual void OnVoltageChanged();
  virtual void OnModeChanged();

  //Receive
  virtual void OnScanDepthChanged();
  virtual void OnAveragingCountChanged();
  virtual void OnTGCMinChanged();
  virtual void OnTGCMaxChanged();
  virtual void OnDataTypeChanged();

  //Beamforming
  virtual void OnPitchChanged();
  virtual void OnReconstructedSamplesChanged();
  virtual void OnReconstructedLinesChanged();
  virtual void OnSpeedOfSoundChanged();

  //Bandpass
  virtual void OnBandpassEnabledChanged();
  virtual void OnLowCutChanged();
  virtual void OnHighCutChanged();

public:
  /**
    * Constructs widget object. All gui control elements will be disabled until
    * QmitkUSAbstractCustomWidget::SetDevice() was called.
    */
  QmitkUSControlsCustomDiPhASDeviceWidget();
  ~QmitkUSControlsCustomDiPhASDeviceWidget();

  /**
    * Getter for the device class of mitk:USDiPhASDevice.
    */
  std::string GetDeviceClass() const override;

  /**
    * Creates new QmitkUSAbstractCustomWidget with the same mitk::USDiPhASDevice
    * and the same mitk::USDiPhASDeviceCustomControls which were set on the
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

private:
  QmitkUSControlsCustomDiPhASDeviceWidget(QWidget *parent);
  Ui::QmitkUSControlsCustomDiPhASDeviceWidget*         ui;
  mitk::USDiPhASDeviceCustomControls::Pointer  m_ControlInterface;
  int m_OldReconstructionLines;
};

#endif // QmitkUSControlsCustomDiPhASDeviceWidget_H