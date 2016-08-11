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

#ifndef MITKUSDiPhASDopplerControls_H_HEADER_INCLUDED_
#define MITKUSDiPhASDopplerControls_H_HEADER_INCLUDED_

#include "mitkUSDiPhASSDKHeader.h"
#include "mitkUSControlInterfaceDoppler.h"

#include <itkObjectFactory.h>

namespace mitk {
  class USDiPhASDevice;
  /**
    * \brief Implementation of mitk::USControlInterfaceDoppler for DiPhAS ultrasound devices.
    * See documentation of mitk::USControlInterfaceBMode for a description of the interface methods.
    *
    * This implementation does nothing, yet!
    */
  class USDiPhASDopplerControls : public USControlInterfaceDoppler
  {
  public:
    mitkClassMacro(USDiPhASDopplerControls, USControlInterfaceDoppler);
    mitkNewMacro1Param(Self, itk::SmartPointer<USDiPhASDevice>);

    virtual void SetIsActive(bool);
    virtual bool GetIsActive();

  protected:
    USDiPhASDopplerControls(itk::SmartPointer<USDiPhASDevice> device);
    virtual ~USDiPhASDopplerControls();

    bool  m_IsActive;
  };
}

#endif // MITKUSDiPhASDopplerControls_H_HEADER_INCLUDED_