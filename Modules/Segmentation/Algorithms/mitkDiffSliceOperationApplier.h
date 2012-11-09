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

#include "SegmentationExports.h"
#include "mitkCommon.h"
#include <mitkOperationActor.h>

#include "mitkDiffSliceOperation.h"
#include <mitkExtractSliceFilter.h>
#include <mitkVtkImageOverwrite.h>

namespace mitk
{
  /** \brief Executes a DiffSliceOperation.
    \sa DiffSliceOperation
  */
  class Segmentation_EXPORT DiffSliceOperationApplier : public OperationActor
  {

  public:

    mitkClassMacro(DiffSliceOperationApplier, OperationActor);

    //itkNewMacro(DiffSliceOperationApplier);

    /** \brief Returns an instance of the class */
    static DiffSliceOperationApplier* GetInstance();

    /** \brief Executes a DiffSliceOperation.
      \sa DiffSliceOperation
      Note:
        Only DiffSliceOperation is supported.
    */
    virtual void ExecuteOperation(Operation* op);


  protected:

    DiffSliceOperationApplier();

    virtual ~DiffSliceOperationApplier();

    //static DiffSliceOperationApplier* s_Instance;

  };
}
#endif