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

#ifndef MITKUSFraunhoferProbesControls_H_HEADER_INCLUDED_
#define MITKUSFraunhoferProbesControls_H_HEADER_INCLUDED_

#include "mitkUSFraunhoferSDKHeader.h"
#include "mitkUSFraunhoferProbe.h"
#include "mitkUSControlInterfaceProbes.h"

#include <itkObjectFactory.h>

namespace mitk {
  class USFraunhoferDevice;

  /**
    * \brief Implementation of mitk::USControlInterfaceProbes for Fraunhofer ultrasound devices.
    * See documentation of mitk::USControlInterfaceProbes for a description of the interface methods.
	* This class has to be implemented for the USDevice but the Fraunhofer API does not support multiple devices.
	* Therefore there will be just one probe at all times.
    */
  class USFraunhoferProbesControls : public USControlInterfaceProbes
  {
  public:
    mitkClassMacro(USFraunhoferProbesControls, USControlInterfaceProbes);
    mitkNewMacro1Param(Self, itk::SmartPointer<USFraunhoferDevice>);

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
      * Fraunhofer device has to be set after constructing by calling
      * mitk::USFraunhoferProbesControls::SetFraunhoferDevice before the
      * object can be used.
      */
    USFraunhoferProbesControls(itk::SmartPointer<USFraunhoferDevice> device);
    virtual ~USFraunhoferProbesControls();

    /**
      * Create collection object (Fraunhofer API) for the API device.
      */
    bool CreateProbesCollection();

    /**
      * Create vector of mitk::USFraunhoferProbe objects from the
      * Fraunhofer API probe collection. Hence
      * mitk::USFraunhoferProbesControls::CreateProbesCollection has to
      * be called before.
      */
    void CreateProbesSet();

    bool                                  m_IsActive;
    unsigned int                          m_SelectedProbeIndex;
    std::vector<USFraunhoferProbe::Pointer>  m_ProbesSet;
    itk::SmartPointer<USFraunhoferDevice>    m_FraunhoferDevice;
  };
}

#endif // MITKUSFraunhoferProbesControls_H_HEADER_INCLUDED_