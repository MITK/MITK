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

#ifndef MITKUSDiPhASDeviceCustomControls_H_HEADER_INCLUDED_
#define MITKUSDiPhASDeviceCustomControls_H_HEADER_INCLUDED_

#include "mitkUSAbstractControlInterface.h"
#include "mitkUSImageVideoSource.h"
#include "mitkUSDevice.h"

#include <itkObjectFactory.h>

#include <functional>
#include <qstring.h>

namespace mitk {
/**
  * \brief Custom controls for mitk::USDiPhASDevice.
  */
class MITKUS_EXPORT USDiPhASDeviceCustomControls : public USAbstractControlInterface
{
public:
  mitkClassMacro(USDiPhASDeviceCustomControls, USAbstractControlInterface);
  mitkNewMacro1Param(Self, itk::SmartPointer<USDevice>);

  /**
    * Activate or deactivate the custom controls. This is just for handling
    * widget visibility in a GUI for example.
    */
  void SetIsActive( bool isActive ) override;

  enum DataType { Image_uChar, Beamformed_Short };

  struct SavingSettings
  {
    bool saveRaw;
    bool saveBeamformed;
  };
  /**
    * \return if this custom controls are currently activated
    */
  bool GetIsActive( ) override;

  virtual void SetCompensateEnergy(bool compensate);
  virtual void SetUseBModeFilter(bool isSet);
  virtual void SetVerticalSpacing(float mm);
  virtual void SetRecord(bool record);  
  virtual void SetScatteringCoefficient(float coeff);
  virtual void SetCompensateScattering(bool compensate);
  virtual void SetSavingSettings(SavingSettings settings);

  //Transmit
  virtual void SetTransmitPhaseLength(double us);
  virtual void SetExcitationFrequency(double MHz);
  virtual void SetTransmitEvents(int events);
  virtual void SetVoltage(int voltage);
  virtual void SetMode(bool interleaved);

  //Receive
  virtual void SetScanDepth(double mm);
  virtual void SetAveragingCount(int count);
  virtual void SetTGCMin(int min);
  virtual void SetTGCMax(int max);
  virtual void SetDataType(DataType type);

  //Beamforming
  virtual void SetPitch(double mm);
  virtual void SetReconstructedSamples(int samples);
  virtual void SetReconstructedLines(int lines);
  virtual void SetSpeedOfSound(int mps);

  //Bandpass
  virtual void SetBandpassEnabled(bool bandpass);
  virtual void SetLowCut(double MHz);
  virtual void SetHighCut(double MHz);

  virtual void passGUIOut(std::function<void(QString)> /*callback*/);
  virtual void SetSilentUpdate(bool silent);
  virtual bool GetSilentUpdate();



protected:
  /**
    * Class needs an mitk::USDevice object for beeing constructed.
    */
  USDiPhASDeviceCustomControls( itk::SmartPointer<USDevice> device );
  ~USDiPhASDeviceCustomControls( ) override;

  bool                          m_IsActive;
  USImageVideoSource::Pointer   m_ImageSource;
  bool                          silentUpdate;

  /** virtual handlers implemented in Device Controls
    */
  virtual void OnSetCompensateEnergy(bool /*compensate*/);
  virtual void OnSetSavingSettings(SavingSettings /*settings*/);
  virtual void OnSetUseBModeFilter(bool /*isSet*/);
  virtual void OnSetRecord(bool /*record*/);
  virtual void OnSetVerticalSpacing(float /*mm*/);
  virtual void OnSetScatteringCoefficient(float /*coeff*/);
  virtual void OnSetCompensateScattering(bool /*compensate*/);
  //Transmit
  virtual void OnSetTransmitPhaseLength(double /*us*/);
  virtual void OnSetExcitationFrequency(double /*MHz*/);
  virtual void OnSetTransmitEvents(int /*events*/);
  virtual void OnSetVoltage(int /*voltage*/);
  virtual void OnSetMode(bool /*interleaved*/);

  //Receive
  virtual void OnSetScanDepth(double /*mm*/);
  virtual void OnSetAveragingCount(int /*count*/);
  virtual void OnSetTGCMin(int /*min*/);
  virtual void OnSetTGCMax(int /*max*/);
  virtual void OnSetDataType(DataType /*type*/);

  //Beamforming
  virtual void OnSetPitch(double /*mm*/);
  virtual void OnSetReconstructedSamples(int /*samples*/);
  virtual void OnSetReconstructedLines(int /*lines*/);
  virtual void OnSetSpeedOfSound(int /*mps*/);

  //Bandpass
  virtual void OnSetBandpassEnabled(bool /*bandpass*/);
  virtual void OnSetLowCut(double /*MHz*/);
  virtual void OnSetHighCut(double /*MHz*/);

};
} // namespace mitk

#endif // MITKUSDiPhASDeviceCustomControls_H_HEADER_INCLUDED_
