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

#ifndef MITKUSTelemedDopplerControls_H_HEADER_INCLUDED_
#define MITKUSTelemedDopplerControls_H_HEADER_INCLUDED_

#include "mitkUSTelemedSDKHeader.h"
#include "mitkUSControlInterfaceDoppler.h"

#include <itkObjectFactory.h>

namespace mitk {
  class USTelemedDopplerControls : public USControlInterfaceDoppler
  {
  public:
    mitkClassMacro(USTelemedDopplerControls, USControlInterfaceDoppler);
    itkNewMacro(Self);

  protected:
    USTelemedDopplerControls();
    virtual ~USTelemedDopplerControls();

    virtual void SetIsActive(bool);
    virtual bool GetIsActive();

    bool  m_IsActive;
  };
}

#endif // MITKUSTelemedDopplerControls_H_HEADER_INCLUDED_