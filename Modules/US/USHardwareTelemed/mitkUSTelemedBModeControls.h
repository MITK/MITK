/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef MITKUSTelemedBModeControls_H_HEADER_INCLUDED_
#define MITKUSTelemedBModeControls_H_HEADER_INCLUDED_

#include "mitkUSTelemedSDKHeader.h"
#include "mitkUSControlInterfaceBMode.h"

#include <itkObjectFactory.h>

namespace mitk {
  class USTelemedDevice;

  /**
    * \brief Implementation of mitk::USControlInterfaceBMode for Telemed ultrasound devices.
    * See documentation of mitk::USControlInterfaceBMode for a description of the interface methods.
    */
  class USTelemedBModeControls : public USControlInterfaceBMode
  {
  public:
    mitkClassMacro(USTelemedBModeControls, USControlInterfaceBMode);
    mitkNewMacro1Param(Self, itk::SmartPointer<USTelemedDevice>);

    /**
      * Scan mode is set to b mode when this controls are activated.
      * All necessary controls are created at the Telemed API.
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

    /**
      * \brief Setter for the IUsgDataView necesary for communicating with the Telemed API.
      * This method is just for internal use of the mitk::USTelemedDevice.
      */
    void SetUsgDataView( Usgfw2Lib::IUsgDataView* );

    void ReinitializeControls( );

  protected:
    USTelemedBModeControls(itk::SmartPointer<USTelemedDevice> device);
    virtual ~USTelemedBModeControls();

    void CreateControls( );
    void ReleaseControls( );

    double GetScanningFrequencyAPI( );

    Usgfw2Lib::IUsgDataView*             m_UsgDataView;         // main SDK object for comminucating with the Telemed API
    Usgfw2Lib::IUsgPower*                m_PowerControl;        // control for scanning power
    Usgfw2Lib::IUsgProbeFrequency2*      m_FrequencyControl;    // control for scanning frequency
    Usgfw2Lib::IUsgDepth*                m_DepthControl;        // control for B mode scanning depth
    Usgfw2Lib::IUsgGain*                 m_GainControl;         // control for B mode scanning gain
    Usgfw2Lib::IUsgRejection2*           m_RejectionControl;    // control for B mode scanning rejection
    Usgfw2Lib::IUsgDynamicRange*         m_DynamicRangeControl; // control for B mode dynamic range

    bool                      m_Active;

    double*                   m_PowerSteps;        // array holding possible power values: [min, max, tick]
    double*                   m_GainSteps;         // array holding possible gains: [min, max, tick]
    double*                   m_RejectionSteps;    // array holding possible rejections: [min, max, tick]
    double*                   m_DynamicRangeSteps; // array holding possible dynamic range values: [min, max, tick]
  };
} // namespace mitk

#endif // MITKUSTelemedBModeControls_H_HEADER_INCLUDED_
