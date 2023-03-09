/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkSemanticRelationException_h
#define mitkSemanticRelationException_h

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

#endif
