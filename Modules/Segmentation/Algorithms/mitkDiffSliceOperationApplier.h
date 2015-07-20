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

#ifndef mitkDiffSliceOpertationApplier_h_Included
#define mitkDiffSliceOpertationApplier_h_Included

#include <MitkSegmentationExports.h>
#include "mitkCommon.h"
#include <mitkOperationActor.h>

namespace mitk
{
  /** \brief Executes a DiffSliceOperation.
    \sa DiffSliceOperation
  */
  class MITKSEGMENTATION_EXPORT DiffSliceOperationApplier : public OperationActor
  {

  public:

    mitkClassMacroNoParent(DiffSliceOperationApplier)

    /** \brief Returns an instance of the class */
    static DiffSliceOperationApplier* GetInstance();

    /** \brief Executes a DiffSliceOperation.
      \sa DiffSliceOperation
      Note:
        Only DiffSliceOperation is supported.
    */
    virtual void ExecuteOperation(Operation* op) override;


  protected:

    DiffSliceOperationApplier();

    virtual ~DiffSliceOperationApplier();
  };
}
#endif
