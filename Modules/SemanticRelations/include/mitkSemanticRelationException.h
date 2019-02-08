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

#ifndef MITKSEMANTICRELATIONEXCEPTION_H
#define MITKSEMANTICRELATIONEXCEPTION_H

// mitk core
#include <mitkExceptionMacro.h>

namespace mitk
{
  class SemanticRelationException : public Exception
  {

  public:

    mitkExceptionClassMacro(SemanticRelationException, Exception);

  };
} // namespace mitk

#endif // MITKSEMANTICRELATIONEXCEPTION_H
