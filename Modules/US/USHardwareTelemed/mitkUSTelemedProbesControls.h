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

#ifndef MITKUSTelemedProbesControls_H_HEADER_INCLUDED_
#define MITKUSTelemedProbesControls_H_HEADER_INCLUDED_

#include "mitkUSTelemedSDKHeader.h"
#include "mitkUSTelemedProbe.h"
#include "mitkUSControlInterfaceProbes.h"

#include <itkObjectFactory.h>

namespace itk {
  template<class T> class SmartPointer;
}

namespace mitk {
  class USTelemedDevice;

  /**
    * \brief Implementation of mitk::USControlInterfaceProbes for Telemed ultrasound devices.
    * See documentation of mitk::USControlInterfaceProbes for a description of the interface methods.
    */
  class USTelemedProbesControls : public USControlInterfaceProbes
  {
  public:
    mitkClassMacro(USTelemedProbesControls, USControlInterfaceProbes);
    itkNewMacro(Self);

    /**
      * Probe informations are fetched on activation. On deactivation there is nothing done.
      */
    virtual void SetIsActive(bool);

    virtual bool GetIsActive();

    virtual std::vector<USProbe::Pointer> GetProbeSet();
    virtual void SelectProbe(unsigned int index);
    virtual void SelectProbe(USProbe::Pointer probe);
    virtual unsigned int GetProbesCount() const;

    /**
      * Setter for the mitk::USTelemedDevice necessary to communicate with the Telemed API.
      */
    void SetTelemedDevice(itk::SmartPointer<USTelemedDevice> telemedDevice);

  protected:
    /**
      * Constructs an empty object.
      * Telemed device has to be set after constructing by calling
      * mitk::USTelemedProbesControls::SetTelemedDevice before the
      * object can be used.
      */
    USTelemedProbesControls();
    virtual ~USTelemedProbesControls();

    /**
      * Create collection object (Telemed API) for the API device.
      */
    bool CreateProbesCollection();

    /**
      * Create vector of mitk::USTelemedProbe objects from the
      * Telemed API probe collection. Hence
      * mitk::USTelemedProbesControls::CreateProbesCollection has to
      * be called before.
      */
    void CreateProbesSet();

    bool                                  m_IsActive;
    std::vector<USTelemedProbe::Pointer>  m_ProbesSet;
    itk::SmartPointer<USTelemedDevice>    m_TelemedDevice;

    IUsgCollection*                       m_ProbesCollection;
    IProbe*                               m_Probe;
  };
}

#endif // MITKUSTelemedProbesControls_H_HEADER_INCLUDED_