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

#ifndef IGTHARDWAREEXCEPTION_H_INCLUDED
#define IGTHARDWAREEXCEPTION_H_INCLUDED

#include "mitkIGTException.h"
#include "mitkExceptionMacro.h"

namespace mitk {
  /**Documentation
  * \brief An object of this class represents an exception of the MITK-IGT module which are releated to the hardware (e.g. connection problems, etc.).
  *
  * \ingroup IGT
  */  class MitkIGT_EXPORT IGTHardwareException : public mitk::IGTException
  {
  public:
    mitkExceptionClassMacro(IGTHardwareException,mitk::IGTException);
  };
} // namespace mitk
#endif
