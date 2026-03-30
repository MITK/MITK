/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkUnstructuredGridSource_h
#define mitkUnstructuredGridSource_h

#include <MitkDataTypesExtExports.h>
#include <mitkBaseDataSource.h>

namespace mitk
{
  class UnstructuredGrid;
  /**
   * \brief Base class for all process objects that produce a mitk::UnstructuredGrid.
   *
   * The generated output is only guaranteed to be up-to-date when Update()
   * is called immediately before accessing the data.
   *
   * \sa UnstructuredGrid, BaseDataSource
   * \ingroup Process
   */
  class MITKDATATYPESEXT_EXPORT UnstructuredGridSource : public BaseDataSource
  {
  public:
    mitkClassMacro(UnstructuredGridSource, BaseDataSource);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    /** \brief The output type of this source. */
    typedef mitk::UnstructuredGrid OutputType;

    mitkBaseDataSourceGetOutputDeclarations

    /**
     * \brief Allocate a new output object.
     *
     * \param[in] idx The output index (currently not evaluated).
     * \return Smart pointer to the newly allocated UnstructuredGrid.
     */
    itk::DataObject::Pointer
    MakeOutput(DataObjectPointerArraySizeType idx) override;

    /**
     * \brief Allocate a new output object by name.
     *
     * \param[in] name The output identifier.
     * \return Smart pointer to the newly allocated UnstructuredGrid.
     */
    itk::DataObject::Pointer MakeOutput(const DataObjectIdentifierType &name) override;

  protected:
    UnstructuredGridSource();

    ~UnstructuredGridSource() override;
  };

} // namespace mitk

#endif
