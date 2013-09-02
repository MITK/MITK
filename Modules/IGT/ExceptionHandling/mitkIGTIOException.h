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

#ifndef IGTIOEXCEPTION_H_INCLUDED
#define IGTIOEXCEPTION_H_INCLUDED

#include "mitkIGTException.h"
#include "mitkExceptionMacro.h"

namespace mitk {
  /**Documentation
  * \brief An object of this class represents an exception of the MITK-IGT module which are releated to the input/output problems (e.g. reading writing files, etc.).
  *
  * \ingroup IGT
  */  class MitkIGT_EXPORT IGTIOException : public mitk::IGTException
  {
  public:
    mitkExceptionClassMacro(IGTIOException,mitk::IGTException);
  };
} // namespace mitk
#endif
