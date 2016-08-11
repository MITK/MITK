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

#ifndef MITKUSDiPhASBModeControls_H_HEADER_INCLUDED_
#define MITKUSDiPhASBModeControls_H_HEADER_INCLUDED_

#include "mitkUSDiPhASSDKHeader.h"
#include "mitkUSControlInterfaceBMode.h"

#include <itkObjectFactory.h>

namespace mitk {
  class USDiPhASDevice;

  /**
    * \brief Implementation of mitk::USControlInterfaceBMode for DiPhAS ultrasound devices.
    * See documentation of mitk::USControlInterfaceBMode for a description of the interface methods.
    */
  class USDiPhASBModeControls : public USControlInterfaceBMode
  {
  public:
    mitkClassMacro(USDiPhASBModeControls, USControlInterfaceBMode);
    mitkNewMacro1Param(Self, itk::SmartPointer<USDiPhASDevice>);

    /**
      * Scan mode is set to b mode when this controls are activated.
      * All necessary controls are created at the DiPhAS API.
      */
    virtual void SetIsActive( bool );

    /**
      * \return true if API controls are created and scan mode is set to b mode
      */
    virtual bool GetIsActive( );

    virtual double GetScanningFrequency( );
    virtual void OnSetScanningFrequency( double frequency );
    virtual std::vector<double> GetScanningFrequencyValues( );

    virtual double GetScanningPower( );
    virtual void OnSetScanningPower( double power );
    virtual double GetScanningPowerMin( );
    virtual double GetScanningPowerMax( );
    virtual double GetScanningPowerTick( );

    virtual double GetScanningDepth( );
    virtual void OnSetScanningDepth( double );
    virtual std::vector<double> GetScanningDepthValues( );

    virtual double GetScanningGain( );
    virtual void OnSetScanningGain( double );
    virtual double GetScanningGainMin( );
    virtual double GetScanningGainMax( );
    virtual double GetScanningGainTick( );

    virtual double GetScanningRejection( );
    virtual void OnSetScanningRejection( double );
    virtual double GetScanningRejectionMin( );
    virtual double GetScanningRejectionMax( );
    virtual double GetScanningRejectionTick( );

    virtual double GetScanningDynamicRange( );
    virtual void OnSetScanningDynamicRange( double );
    virtual double GetScanningDynamicRangeMin( );
    virtual double GetScanningDynamicRangeMax( );
    virtual double GetScanningDynamicRangeTick( );

  protected:
    USDiPhASBModeControls(itk::SmartPointer<USDiPhASDevice> device);
    virtual ~USDiPhASBModeControls();

    bool                      m_Active;

    double*                   m_PowerSteps;        // array holding possible power values: [min, max, tick]
    double*                   m_GainSteps;         // array holding possible gains: [min, max, tick]
    double*                   m_RejectionSteps;    // array holding possible rejections: [min, max, tick]
    double*                   m_DynamicRangeSteps; // array holding possible dynamic range values: [min, max, tick]
  };
} // namespace mitk

#endif // MITKUSDiPhASBModeControls_H_HEADER_INCLUDED_