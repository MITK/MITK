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

#ifndef MITKUSControlInterfaceProbes_H_HEADER_INCLUDED_
#define MITKUSControlInterfaceProbes_H_HEADER_INCLUDED_

#include "mitkUSAbstractControlInterface.h"
#include "mitkUSProbe.h"

namespace mitk {

  class MitkUS_EXPORT USControlInterfaceProbes : public USAbstractControlInterface
  {
  public:
    mitkClassMacro(USControlInterfaceProbes, USAbstractControlInterface);

    virtual std::vector<USProbe::Pointer> GetProbeSet() = 0;
    virtual void SelectProbe(unsigned int index) = 0;
    virtual void SelectProbe(USProbe::Pointer probe) = 0;

  protected:
    USControlInterfaceProbes();
    virtual ~USControlInterfaceProbes();
  };

} // namespace mitk

#endif // MITKUSControlInterfaceProbes_H_HEADER_INCLUDED_
