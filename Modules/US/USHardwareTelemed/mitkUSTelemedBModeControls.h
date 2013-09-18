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

#ifndef MITKUSTelemedBModeControls_H_HEADER_INCLUDED_
#define MITKUSTelemedBModeControls_H_HEADER_INCLUDED_

#include "mitkUSTelemedSDKHeader.h"
#include "mitkUSControlInterfaceBMode.h"

#include <itkObjectFactory.h>

namespace mitk {
  /**
    * \brief Implementation of mitk::USControlInterfaceBMode for Telemed ultrasound devices.
    * See documentation of mitk::USControlInterfaceBMode for a description of the interface methods.
    */
  class USTelemedBModeControls : public USControlInterfaceBMode
  {
  public:
    mitkClassMacro(USTelemedBModeControls, USControlInterfaceBMode);
    itkNewMacro(Self);

    /**
      * Scan mode is set to b mode when this controls are activated.
      * All necessary controls are created at the Telemed API.
      */
    virtual void SetIsActive( bool );

    /**
      * \return true if API controls are created and scan mode is set to b mode
      */
    virtual bool GetIsActive( );

    virtual double GetScanningDepth( );
    virtual void SetScanningDepth( double );
    virtual std::vector<double> GetScanningDepthValues( );

    virtual double GetScanningGain( );
    virtual void SetScanningGain( double );
    virtual double GetScanningGainMin( );
    virtual double GetScanningGainMax( );
    virtual double GetScanningGainTick( );

    virtual double GetScanningRejection( );
    virtual void SetScanningRejection( double );
    virtual double GetScanningRejectionMin( );
    virtual double GetScanningRejectionMax( );
    virtual double GetScanningRejectionTick( );

    /**
      * \brief Setter for the IUsgDataView necesary for communicating with the Telemed API.
      * This method is just for internal use of the mitk::USTelemedDevice.
      */
    void SetUsgDataView( IUsgDataView* );

  protected:
    USTelemedBModeControls();
    virtual ~USTelemedBModeControls();

    void CreateControls( );
    void ReleaseControls( );

    IUsgDataView*             m_UsgDataView; // main SDK object for comminucating with the Telemed API
    IUsgDepth*                m_DepthControl; // control for B mode scanning depth
    IUsgGain*                 m_GainControl; // control for B mode scanning gain
    IUsgRejection2*           m_RejectionControl; // control for B mode scanning rejection

    bool                      m_Active;

    double*                   m_GainSteps;      // array holding possible gains: [min, max, tick]
    double*                   m_RejectionSteps; // array holding possible rejections: [min, max, tick]
  };
} // namespace mitk

#endif // MITKUSTelemedBModeControls_H_HEADER_INCLUDED_