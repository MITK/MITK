/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkPointSetSource_h
#define mitkPointSetSource_h

#include "mitkBaseDataSource.h"
#include "mitkPointSet.h"

namespace mitk
{
  /**
   * @brief Superclass of all classes generating point sets (instances of class
   * mitk::PointSet) as output.
   *
   * In itk and vtk the generated result of a ProcessObject is only guaranteed
   * to be up-to-date, when Update() of the ProcessObject or the generated
   * DataObject is called immediately before access of the data stored in the
   * DataObject.
   * @ingroup Process
   */
  class MITKCORE_EXPORT PointSetSource : public BaseDataSource
  {
  public:
    mitkClassMacro(PointSetSource, BaseDataSource);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

        typedef PointSet OutputType;

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
     * Once all the subclasses of ProcessObject provide an appropriate
     * MakeOutput(), then ProcessObject::MakeOutput() can be made pure
     * virtual.
     */
    itk::DataObject::Pointer MakeOutput(const DataObjectIdentifierType &name) override;

  protected:
    PointSetSource();

    ~PointSetSource() override;
  };
}
#endif
