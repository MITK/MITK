/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkContourModelSetSource_h
#define mitkContourModelSetSource_h

#include <mitkBaseDataSource.h>
#include <mitkContourModelSet.h>
#include <MitkContourModelExports.h>

namespace mitk
{
  /** \brief Base class for all pipeline sources that produce mitk::ContourModelSet outputs.
   *
   * Provides the output type declaration and default MakeOutput implementation
   * for creating ContourModelSet instances in the ITK pipeline framework.
   *
   * \sa ContourModelSet, ContourModelSource
   * \ingroup MitkContourModelModule
   */
  class MITKCONTOURMODEL_EXPORT ContourModelSetSource : public BaseDataSource
  {
  public:
    mitkClassMacro(ContourModelSetSource, BaseDataSource);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

      /** \brief The output data type produced by this source. */
      typedef ContourModelSet OutputType;

    /** \brief Smart pointer type for the output. */
    typedef OutputType::Pointer OutputTypePointer;

    mitkBaseDataSourceGetOutputDeclarations

      /** \brief Allocate a new output object at the given index.
       * \param[in] idx The output index (currently not evaluated).
       * \return A new ContourModelSet instance wrapped in a DataObject pointer.
       */
      itk::DataObject::Pointer
      MakeOutput(DataObjectPointerArraySizeType idx) override;

    /** \brief Allocate a new output object by name.
     * \param[in] name Identifier for the output.
     * \return A new ContourModelSet instance wrapped in a DataObject pointer.
     */
    itk::DataObject::Pointer MakeOutput(const DataObjectIdentifierType &name) override;

  protected:
    ContourModelSetSource();

    ~ContourModelSetSource() override;
  };
}
#endif
