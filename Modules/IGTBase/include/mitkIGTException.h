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

#ifndef IGTEXCEPTION_H_INCLUDED
#define IGTEXCEPTION_H_INCLUDED

#include <MitkIGTBaseExports.h>
#include <mitkException.h>
#include "mitkExceptionMacro.h"

namespace mitk {
/**Documentation
* \brief An object of this class represents an exception of the MITK-IGT module.
*
* \ingroup IGT
*/  class MITKIGTBASE_EXPORT IGTException : public mitk::Exception
{
  public:
    mitkExceptionClassMacro(IGTException,mitk::Exception);
  };
} // namespace mitk
#endif
