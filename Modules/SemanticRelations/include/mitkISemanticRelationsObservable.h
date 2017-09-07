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

#ifndef MITKISEMANTICRELATIONSOBSERVABLE_H
#define MITKISEMANTICRELATIONSOBSERVABLE_H

#include "mitkISemanticRelationsObserver.h"

#include "mitkSemanticTypes.h"

namespace mitk
{
  class ISemanticRelationsObservable
  {
  public:

    virtual void AddObserver(ISemanticRelationsObserver* observer) = 0;
    virtual void RemoveObserver(ISemanticRelationsObserver* observer) = 0;
    virtual void NotifyObserver(const mitk::SemanticTypes::CaseID& caseID) = 0;

  }; // class ISemanticRelationsObservable
} // namespace mitk

#endif // MITKISEMANTICRELATIONSOBSERVABLE_H
