/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkIGTIOException_h
#define mitkIGTIOException_h

#include "mitkIGTException.h"
#include "mitkExceptionMacro.h"

namespace mitk {
  /**Documentation
  * \brief An object of this class represents an exception of the MITK-IGT module which are releated to the input/output problems (e.g. reading writing files, etc.).
  *
  * \ingroup IGT
  */  class MITKIGT_EXPORT IGTIOException : public mitk::IGTException
  {
  public:
    mitkExceptionClassMacro(IGTIOException,mitk::IGTException);
  };
} // namespace mitk
#endif
