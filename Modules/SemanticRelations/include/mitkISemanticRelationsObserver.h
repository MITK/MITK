/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef MITKISEMANTICRELATIONSOBSERVER_H
#define MITKISEMANTICRELATIONSOBSERVER_H

#include "mitkSemanticTypes.h"

namespace mitk
{
  /*
  * @brief This interface declares a functions each observer has to implement
  *        in order to be notified in the 'Observer pattern' sense.
  */
  class ISemanticRelationsObserver
  {
  public:
    /*
    * @brief Updates the concrete observer.
    *        The caseID can be used to get access to a certain patient (case),
    *        whose data should be used for updating.
    *
    * @param caseID    The current case ID to identify the currently active patient / case.
    */
    virtual void Update(const mitk::SemanticTypes::CaseID& caseID) = 0;

  }; // class ISemanticRelationsObserver
} // namespace mitk

#endif // MITKISEMANTICRELATIONSOBSERVER_H
