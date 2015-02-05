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

#ifndef MITKUSControlInterfaceDoppler_H_HEADER_INCLUDED_
#define MITKUSControlInterfaceDoppler_H_HEADER_INCLUDED_

#include "mitkUSAbstractControlInterface.h"

namespace mitk {
  /**
    * \brief Interface defining methods for scanning mode doppler of ultrasound devices.
    * There are no methods defined, yet! At the moment, this is just an empty
    * interface.
    */
  class MITKUS_EXPORT USControlInterfaceDoppler : public USAbstractControlInterface
  {
  public:
    mitkClassMacro(USControlInterfaceDoppler, USAbstractControlInterface);

  protected:
    USControlInterfaceDoppler( itk::SmartPointer<USDevice> device );
    virtual ~USControlInterfaceDoppler( );
  };
} // namespace mitk

#endif // MITKUSControlInterfaceDoppler_H_HEADER_INCLUDED_
