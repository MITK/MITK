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

#ifndef MITKISEMANTICRELATIONSOBSERVER_H
#define MITKISEMANTICRELATIONSOBSERVER_H

#include "mitkSemanticTypes.h"

namespace mitk
{
  class ISemanticRelationsObserver
  {
  public:

    virtual void Update(const mitk::SemanticTypes::CaseID& caseID) = 0;

  }; // class ISemanticRelationsObserver
} // namespace mitk

#endif // MITKISEMANTICRELATIONSOBSERVER_H
