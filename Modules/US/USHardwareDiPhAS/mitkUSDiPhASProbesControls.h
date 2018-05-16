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

#ifndef MITKUSDiPhASProbesControls_H_HEADER_INCLUDED_
#define MITKUSDiPhASProbesControls_H_HEADER_INCLUDED_

#include "Framework.IBMT.US.CWrapper.h"
#include "mitkUSDiPhASProbe.h"
#include "mitkUSControlInterfaceProbes.h"

#include <itkObjectFactory.h>

namespace mitk {
  class USDiPhASDevice;

  /**
    * \brief Implementation of mitk::USControlInterfaceProbes for DiPhAS ultrasound devices.
    * See documentation of mitk::USControlInterfaceProbes for a description of the interface methods.
	* This class has to be implemented for the USDevice but the DiPhAS API does not support multiple devices.
	* Therefore there will be just one probe at all times.
    */
  class USDiPhASProbesControls : public USControlInterfaceProbes
  {
  public:
    mitkClassMacro(USDiPhASProbesControls, USControlInterfaceProbes);
    mitkNewMacro1Param(Self, itk::SmartPointer<USDiPhASDevice>);

    /**
      * Probe informations are fetched on activation. On deactivation there is nothing done.
      */
    virtual void SetIsActive(bool);

    virtual bool GetIsActive();

    virtual std::vector<USProbe::Pointer> GetProbeSet();
    virtual void OnSelectProbe(unsigned int index);
    virtual void OnSelectProbe(USProbe::Pointer probe);
    virtual USProbe::Pointer GetSelectedProbe();
    virtual unsigned int GetProbesCount() const;

    void ProbeRemoved(unsigned int index);
    void ProbeAdded(unsigned int index);

  protected:
    /**
      * Constructs an empty object.
      * DiPhAS device has to be set after constructing by calling
      * mitk::USDiPhASProbesControls::SetDiPhASDevice before the
      * object can be used.
      */
    USDiPhASProbesControls(itk::SmartPointer<USDiPhASDevice> device);
    virtual ~USDiPhASProbesControls();

    /**
      * Create collection object (DiPhAS API) for the API device.
      */
    bool CreateProbesCollection();

    /**
      * Create vector of mitk::USDiPhASProbe objects from the
      * DiPhAS API probe collection. Hence
      * mitk::USDiPhASProbesControls::CreateProbesCollection has to
      * be called before.
      */
    void CreateProbesSet();

    bool                                  m_IsActive;
    unsigned int                          m_SelectedProbeIndex;
    std::vector<USDiPhASProbe::Pointer>  m_ProbesSet;
    itk::SmartPointer<USDiPhASDevice>    m_DiPhASDevice;
  };
}

#endif // MITKUSDiPhASProbesControls_H_HEADER_INCLUDED_