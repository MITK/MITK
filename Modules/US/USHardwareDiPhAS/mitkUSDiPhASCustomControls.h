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

#ifndef MITKUSDiPhASCustomControls_H_HEADER_INCLUDED_
#define MITKUSDiPhASCustomControls_H_HEADER_INCLUDED_

#include "mitkUSDiPhASDeviceCustomControls.h"
#include "mitkUSDevice.h"
#include "mitkUSDiPhASDevice.h"
#include "Framework.IBMT.US.CWrapper.h"

#include <itkObjectFactory.h>

namespace mitk {
/**
  * \brief Custom controls for mitk::USDiPhASDevice.
  */
class USDiPhASDevice;
class USDiPhASCustomControls : public USDiPhASDeviceCustomControls
{
public:
  mitkClassMacro(USDiPhASCustomControls, USAbstractControlInterface);
  mitkNewMacro1Param(Self, mitk::USDiPhASDevice*);

  typedef USDiPhASDeviceCustomControls::DataType DataType;
  typedef USDiPhASDeviceCustomControls::SavingSettings SavingSettings;
  /**
    * Activate or deactivate the custom controls. This is just for handling
    * widget visibility in a GUI for example.
    */
  virtual void SetIsActive( bool isActive ) override;

  /**
    * \return if this custom controls are currently activated
    */
  virtual bool GetIsActive( ) override;

  virtual void passGUIOut(std::function<void(QString)> callback) override;

  BeamformingParametersPlaneWaveCompound   parametersPW;
  BeamformingParametersInterleaved_OA_US parametersOSUS;

protected:
  /**
    * Class needs an mitk::USDiPhASDevice object for beeing constructed.
    * This object's ScanMode will be manipulated by the custom controls methods.
    */
  USDiPhASCustomControls(USDiPhASDevice* device);
  virtual ~USDiPhASCustomControls( );

  bool                          m_IsActive;
  USImageVideoSource::Pointer   m_ImageSource;
  USDiPhASDevice*               m_device;
  int                           currentBeamformingAlgorithm;

  /** handlers for value changes
  */
  virtual void OnSetCompensateEnergy(bool compensate) override;
  virtual void OnSetUseBModeFilter(bool isSet) override;
  virtual void OnSetRecord(bool record) override;
  virtual void OnSetVerticalSpacing(float mm) override;
  virtual void OnSetScatteringCoefficient(float coeff) override;
  virtual void OnSetCompensateScattering(bool compensate) override;
  virtual void OnSetSavingSettings(SavingSettings settings) override;

  //Transmit
  virtual void OnSetTransmitPhaseLength(double us) override;
  virtual void OnSetExcitationFrequency(double MHz) override;
  virtual void OnSetTransmitEvents(int events) override;
  virtual void OnSetVoltage(int voltage) override;
  virtual void OnSetMode(bool interleaved) override;

  //Receive
  virtual void OnSetScanDepth(double mm) override;
  virtual void OnSetAveragingCount(int count) override;
  virtual void OnSetTGCMin(int min) override;
  virtual void OnSetTGCMax(int max) override;
  virtual void OnSetDataType(DataType type) override;

  //Beamforming
  virtual void OnSetPitch(double mm) override;
  virtual void OnSetReconstructedSamples(int samples) override;
  virtual void OnSetReconstructedLines(int lines) override;
  virtual void OnSetSpeedOfSound(int mps) override;

  //Bandpass
  virtual void OnSetBandpassEnabled(bool bandpass) override;
  virtual void OnSetLowCut(double MHz) override;
  virtual void OnSetHighCut(double MHz) override;
};
} // namespace mitk

#endif // MITKUSDiPhASCustomControls_H_HEADER_INCLUDED_