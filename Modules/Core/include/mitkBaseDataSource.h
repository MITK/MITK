/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkBaseDataSource_h
#define mitkBaseDataSource_h

#include <MitkCoreExports.h>
#include <itkProcessObject.h>
#include <mitkCommon.h>

#define mitkBaseDataSourceGetOutputDeclarations                                                                        \
  OutputType *GetOutput();                                                                                             \
  const OutputType *GetOutput() const;                                                                                 \
  OutputType *GetOutput(DataObjectPointerArraySizeType idx);                                                           \
  const OutputType *GetOutput(DataObjectPointerArraySizeType idx) const;

#define mitkBaseDataSourceGetOutputDefinitions(className)                                                              \
  className::OutputType *className::GetOutput(void)                                                                    \
  {                                                                                                                    \
    return itkDynamicCastInDebugMode<OutputType *>(this->GetPrimaryOutput());                                          \
  }                                                                                                                    \
  const className::OutputType *className::GetOutput(void) const                                                        \
  {                                                                                                                    \
    return itkDynamicCastInDebugMode<const OutputType *>(this->GetPrimaryOutput());                                    \
  }                                                                                                                    \
  className::OutputType *className::GetOutput(DataObjectPointerArraySizeType idx)                                      \
  {                                                                                                                    \
    OutputType *out = dynamic_cast<OutputType *>(this->ProcessObject::GetOutput(idx));                                 \
    if (out == nullptr && this->ProcessObject::GetOutput(idx) != nullptr)                                                    \
    {                                                                                                                  \
      itkWarningMacro(<< "Unable to convert output number " << idx << " to type " << typeid(OutputType).name());       \
    }                                                                                                                  \
    return out;                                                                                                        \
  }                                                                                                                    \
  const className::OutputType *className::GetOutput(DataObjectPointerArraySizeType idx) const                          \
  {                                                                                                                    \
    const OutputType *out = dynamic_cast<const OutputType *>(this->ProcessObject::GetOutput(idx));                     \
    if (out == nullptr && this->ProcessObject::GetOutput(idx) != nullptr)                                                    \
    {                                                                                                                  \
      itkWarningMacro(<< "Unable to convert output number " << idx << " to type " << typeid(OutputType).name());       \
    }                                                                                                                  \
    return out;                                                                                                        \
  }

namespace mitk
{
  class BaseData;

  /**
   * \brief Superclass of all classes generating some kind of mitk::BaseData.
   *
   * In ITK and VTK the generated result of a ProcessObject is only guaranteed
   * to be up-to-date when Update() of the ProcessObject or the generated
   * DataObject is called immediately before access of the data stored in the
   * DataObject. This is also true for subclasses of mitk::BaseProcess. But
   * many of the subclasses of mitk::BaseProcess define additional access
   * functions to the generated output that guarantee an up-to-date result; see
   * for example mitk::ImageSource.
   *
   * \ingroup Process
   */
  class MITKCORE_EXPORT BaseDataSource : public itk::ProcessObject
  {
  public:
    mitkClassMacroItkParent(BaseDataSource, itk::ProcessObject);

      /** \brief Output data type. */
      typedef BaseData OutputType;

    /** \brief Identifier type for named data objects. */
    typedef itk::DataObject::DataObjectIdentifierType DataObjectIdentifierType;

    mitkBaseDataSourceGetOutputDeclarations

      /**
       * \brief Graft the specified BaseData onto this BaseDataSource's output.
       *
       * This method is used when a process object is implemented using a
       * mini-pipeline defined in its GenerateData() method. The usage is:
       *
       * \code
       *    // setup the mini-pipeline to process the input to this filter
       *    firstFilterInMiniPipeline->SetInput( this->GetInput() );
       *
       *    // setup the mini-pipeline to calculate the correct regions
       *    // and write to the appropriate bulk data block
       *    lastFilterInMiniPipeline->GraftOutput( this->GetOutput(0) );
       *
       *    // execute the mini-pipeline
       *    lastFilterInMiniPipeline->Update();
       *
       *    // graft the mini-pipeline output back onto this filter's output.
       *    // this is needed to get the appropriate regions passed back.
       *    this->GraftOutput( lastFilterInMiniPipeline->GetOutput(0) );
       * \endcode
       *
       * \param[in] output  The BaseData object to graft onto this source's primary output.
       */
      virtual void
      GraftOutput(OutputType *output);

    /**
     * \brief Graft the specified base data object onto a named output.
     *
     * Similar to the GraftOutput method except it allows you to specify
     * which output is affected by name.
     *
     * \param[in] key     The identifier of the output to graft onto.
     * \param[in] output  The BaseData object to graft.
     *
     * \sa GraftOutput
     */
    virtual void GraftOutput(const DataObjectIdentifierType &key, OutputType *output);

    /**
     * \brief Graft the specified base data object onto this BaseDataSource's idx'th output.
     *
     * Similar to GraftOutput except it allows you to specify which output is
     * affected by index. The specified index must be a valid output number
     * (less than ProcessObject::GetNumberOfOutputs()).
     *
     * \param[in] idx     The zero-based index of the output to graft onto.
     * \param[in] output  The BaseData object to graft.
     *
     * \sa GraftOutput
     */
    virtual void GraftNthOutput(unsigned int idx, OutputType *output);

    /**
     * \brief Create the output data object for the given index.
     *
     * \sa itk::ProcessObject::MakeOutput(DataObjectPointerArraySizeType)
     */
    DataObjectPointer MakeOutput(DataObjectPointerArraySizeType idx) override = 0;

    /**
     * \brief Create the output data object for the given name.
     *
     * \sa itk::ProcessObject::MakeOutput(const DataObjectIdentifierType&)
     */
    DataObjectPointer MakeOutput(const DataObjectIdentifierType &name) override = 0;

    /**
     * \brief Check whether the pipeline is currently executing.
     *
     * Accesses itk::ProcessObject::m_Updating, which prevents infinite
     * recursion when pipelines have loops.
     *
     * \return True if the pipeline is currently executing, false otherwise.
     *
     * \sa itk::ProcessObject::m_Updating
     */
    bool Updating() const;

  protected:
    /** \brief Constructor. */
    BaseDataSource();

    /** \brief Destructor. */
    ~BaseDataSource() override;

    // purposely not implemented
    static Pointer New();
  };

} // namespace mitk

#endif
