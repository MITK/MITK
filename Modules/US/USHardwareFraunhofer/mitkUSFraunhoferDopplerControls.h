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

#ifndef MITKUSFraunhoferDopplerControls_H_HEADER_INCLUDED_
#define MITKUSFraunhoferDopplerControls_H_HEADER_INCLUDED_

#include "mitkUSFraunhoferSDKHeader.h"
#include "mitkUSControlInterfaceDoppler.h"

#include <itkObjectFactory.h>

namespace mitk {
  class USFraunhoferDevice;
  /**
    * \brief Implementation of mitk::USControlInterfaceDoppler for Fraunhofer ultrasound devices.
    * See documentation of mitk::USControlInterfaceBMode for a description of the interface methods.
    *
    * This implementation does nothing, yet!
    */
  class USFraunhoferDopplerControls : public USControlInterfaceDoppler
  {
  public:
    mitkClassMacro(USFraunhoferDopplerControls, USControlInterfaceDoppler);
    mitkNewMacro1Param(Self, itk::SmartPointer<USFraunhoferDevice>);

    virtual void SetIsActive(bool);
    virtual bool GetIsActive();

  protected:
    USFraunhoferDopplerControls(itk::SmartPointer<USFraunhoferDevice> device);
    virtual ~USFraunhoferDopplerControls();

    bool  m_IsActive;
  };
}

#endif // MITKUSFraunhoferDopplerControls_H_HEADER_INCLUDED_