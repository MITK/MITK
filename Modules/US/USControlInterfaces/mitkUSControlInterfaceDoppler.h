/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkUSControlInterfaceDoppler_h
#define mitkUSControlInterfaceDoppler_h

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
    ~USControlInterfaceDoppler( ) override;
  };
} // namespace mitk

#endif
