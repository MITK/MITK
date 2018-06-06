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
  /*
  * @brief This interface declares three functions each observable subject has to implement
  *        in order to be observed in the 'Observer pattern' sense.
  *        The concrete observable class has to store its observer.
  */
  class ISemanticRelationsObservable
  {
  public:
    /*
    * @brief Adds the given concrete observer to a container that holds all currently registered observer.
    *
    * @par observer   The concrete observer to register.
    */
    virtual void AddObserver(ISemanticRelationsObserver* observer) = 0;
    /*
    * @brief Removes the given concrete observer from the container that holds all currently registered observer.
    *
    * @par observer   The concrete observer to unregister.
    */
    virtual void RemoveObserver(ISemanticRelationsObserver* observer) = 0;
    /*
    * @brief Updates all concrete observer in the container that holds all currently registered observer.
    *        The caseID can be used to only update the observer, if the caseID fulfills a certain condition.
    *
    * @par  caseID    A caseID that identifies the currently active patient / case.
    */
    virtual void NotifyObserver(const mitk::SemanticTypes::CaseID& caseID) const = 0;

  }; // class ISemanticRelationsObservable
} // namespace mitk

#endif // MITKISEMANTICRELATIONSOBSERVABLE_H
