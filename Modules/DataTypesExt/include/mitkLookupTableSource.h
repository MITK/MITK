/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkLookupTableSource_h
#define mitkLookupTableSource_h

#include <MitkDataTypesExtExports.h>
#include <mitkCommon.h>
#include <mitkLookupTable.h>

#include <itkProcessObject.h>

namespace mitk
{
  /**
   * \brief Base class for all process objects that produce a mitk::LookupTable as output.
   *
   * It is assumed that LookupTableSource subclasses do not support streaming,
   * i.e. the requested region is always the largest possible region.
   *
   * \sa LookupTable
   * \ingroup Process
   */
  class MITKDATATYPESEXT_EXPORT LookupTableSource : public itk::ProcessObject
  {
  public:
    mitkClassMacroItkParent(LookupTableSource, itk::ProcessObject);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    /** \brief The output type of this source. */
    typedef mitk::LookupTable OutputType;

    /** \brief Smart pointer to the output type. */
    typedef OutputType::Pointer OutputTypePointer;

    /** \brief Generic smart pointer to a data object. */
    typedef itk::DataObject::Pointer DataObjectPointer;

    /**
     * \brief Allocate a new output object.
     *
     * \param[in] idx The output index (currently not evaluated).
     * \return Smart pointer to the newly allocated LookupTable.
     */
    itk::DataObject::Pointer MakeOutput(DataObjectPointerArraySizeType idx) override;

    /**
     * \brief Allocate a new output object by name.
     *
     * \param[in] name The output identifier.
     * \return Smart pointer to the newly allocated LookupTable.
     */
    itk::DataObject::Pointer MakeOutput(const DataObjectIdentifierType &name) override;

    /**
     * \brief Generate the input requested region.
     *
     * Delegates to the superclass implementation.
     */
    void GenerateInputRequestedRegion() override;

    /**
     * \brief Graft the given output onto this source.
     *
     * \param[in] output The LookupTable to graft.
     */
    virtual void GraftOutput(OutputType *output);

    /**
     * \brief Get the primary output.
     * \return Pointer to the output LookupTable.
     */
    virtual OutputType *GetOutput();

    /** \brief Get the primary output (const). */
    virtual const OutputType *GetOutput() const;

    /**
     * \brief Get an output by index.
     * \param[in] idx The output index.
     * \return Pointer to the output LookupTable.
     */
    virtual OutputType *GetOutput(DataObjectPointerArraySizeType idx);

    /** \brief Get an output by index (const). */
    virtual const OutputType *GetOutput(DataObjectPointerArraySizeType idx) const;

  protected:
    LookupTableSource();
    ~LookupTableSource() override;
  };

} // namespace mitk

#endif
