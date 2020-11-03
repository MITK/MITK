/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef MITKUSTelemedDopplerControls_H_HEADER_INCLUDED_
#define MITKUSTelemedDopplerControls_H_HEADER_INCLUDED_

#include "mitkUSTelemedSDKHeader.h"
#include "mitkUSControlInterfaceDoppler.h"

#include <itkObjectFactory.h>

namespace mitk {
  class USTelemedDevice;
  /**
    * \brief Implementation of mitk::USControlInterfaceDoppler for Telemed ultrasound devices.
    * See documentation of mitk::USControlInterfaceBMode for a description of the interface methods.
    *
    * This implementation does nothing, yet!
    */
  class USTelemedDopplerControls : public USControlInterfaceDoppler
  {
  public:
    mitkClassMacro(USTelemedDopplerControls, USControlInterfaceDoppler);
    mitkNewMacro1Param(Self, itk::SmartPointer<USTelemedDevice>);

    virtual void SetIsActive(bool);
    virtual bool GetIsActive();

  protected:
    USTelemedDopplerControls(itk::SmartPointer<USTelemedDevice> device);
    virtual ~USTelemedDopplerControls();

    bool  m_IsActive;
  };
}

#endif // MITKUSTelemedDopplerControls_H_HEADER_INCLUDED_
