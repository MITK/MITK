/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef IGTEXCEPTION_H_INCLUDED
#define IGTEXCEPTION_H_INCLUDED

#include <MitkIGTExports.h>
#include <mitkException.h>
#include "mitkExceptionMacro.h"

namespace mitk {
  /**Documentation
  * \brief An object of this class represents an exception of the MITK-IGT module.
  *
  * \ingroup IGT
  */  class MITKIGT_EXPORT IGTException : public mitk::Exception //perhaps this should later derive from mitk::Exception (if there is an exception handling for complete MITK
  {
  public:

    mitkExceptionClassMacro(IGTException,mitk::Exception);

  };
} // namespace mitk
#endif
