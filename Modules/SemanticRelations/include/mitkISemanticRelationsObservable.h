/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkISemanticRelationsObservable_h
#define mitkISemanticRelationsObservable_h

#include "mitkISemanticRelationsObserver.h"

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
    * @param observer   The concrete observer to register.
    */
    virtual void AddObserver(ISemanticRelationsObserver* observer) = 0;
    /*
    * @brief Removes the given concrete observer from the container that holds all currently registered observer.
    *
    * @param observer   The concrete observer to unregister.
    */
    virtual void RemoveObserver(ISemanticRelationsObserver* observer) = 0;
    /*
    * @brief Updates all concrete observer in the container that holds all currently registered observer.
    *        The caseID can be used to only update the observer, if the caseID fulfills a certain condition.
    *
    * @param  caseID    A caseID that identifies the currently active patient / case.
    */
    virtual void NotifyObserver(const SemanticTypes::CaseID& caseID) const = 0;

  }; // class ISemanticRelationsObservable
} // namespace mitk

#endif
