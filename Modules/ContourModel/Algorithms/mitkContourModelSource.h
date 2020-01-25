/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef _MITK_CONTOURMODEL_SOURCE_H
#define _MITK_CONTOURMODEL_SOURCE_H

#include "mitkBaseDataSource.h"
#include "mitkContourModel.h"
#include <MitkContourModelExports.h>

namespace mitk
{
  /**
  * @brief Superclass of all classes generating ContourModels.
  * @ingroup MitkContourModelModule
  */
  class MITKCONTOURMODEL_EXPORT ContourModelSource : public BaseDataSource
  {
  public:
    mitkClassMacro(ContourModelSource, BaseDataSource);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self)

      typedef ContourModel OutputType;

    typedef OutputType::Pointer OutputTypePointer;

    mitkBaseDataSourceGetOutputDeclarations

      /**
       * Allocates a new output object and returns it. Currently the
       * index idx is not evaluated.
       * @param idx the index of the output for which an object should be created
       * @returns the new object
       */
      itk::DataObject::Pointer
      MakeOutput(DataObjectPointerArraySizeType idx) override;

    /**
     * This is a default implementation to make sure we have something.
     * Once all the subclasses of ProcessObject provide an appopriate
     * MakeOutput(), then ProcessObject::MakeOutput() can be made pure
     * virtual.
     */
    itk::DataObject::Pointer MakeOutput(const DataObjectIdentifierType &name) override;

  protected:
    ContourModelSource();

    ~ContourModelSource() override;
  };
}
#endif // #_MITK_CONTOURMODEL_SOURCE_H
