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


#ifndef MITK_BASE_DATA_SOURCE_H
#define MITK_BASE_DATA_SOURCE_H

#include <MitkCoreExports.h>
#include <mitkCommon.h>
#include <itkProcessObject.h>

#define mitkBaseDataSourceGetOutputDeclarations                          \
  OutputType* GetOutput();                                               \
  const OutputType* GetOutput() const;                                   \
  OutputType* GetOutput(DataObjectPointerArraySizeType idx);             \
  const OutputType* GetOutput(DataObjectPointerArraySizeType idx) const;

#define mitkBaseDataSourceGetOutputDefinitions(className)                                                              \
  className::OutputType* className::GetOutput(void)                                                                    \
  {                                                                                                                    \
    return itkDynamicCastInDebugMode<OutputType*>( this->GetPrimaryOutput() );                                         \
  }                                                                                                                    \
  const className::OutputType* className::GetOutput(void) const                                                        \
  {                                                                                                                    \
    return itkDynamicCastInDebugMode<const OutputType*>( this->GetPrimaryOutput() );                                   \
  }                                                                                                                    \
  className::OutputType* className::GetOutput(DataObjectPointerArraySizeType idx)                                      \
  {                                                                                                                    \
    OutputType* out = dynamic_cast<OutputType*>( this->ProcessObject::GetOutput(idx) );                                \
    if ( out == NULL && this->ProcessObject::GetOutput(idx) != NULL )                                                  \
    {                                                                                                                  \
      itkWarningMacro (<< "Unable to convert output number " << idx << " to type " <<  typeid( OutputType ).name () ); \
    }                                                                                                                  \
    return out;                                                                                                        \
  }                                                                                                                    \
  const className::OutputType* className::GetOutput(DataObjectPointerArraySizeType idx) const                          \
  {                                                                                                                    \
    const OutputType* out = dynamic_cast<const OutputType*>( this->ProcessObject::GetOutput(idx) );                    \
    if ( out == NULL && this->ProcessObject::GetOutput(idx) != NULL )                                                  \
    {                                                                                                                  \
      itkWarningMacro (<< "Unable to convert output number " << idx << " to type " <<  typeid( OutputType ).name () ); \
    }                                                                                                                  \
    return out;                                                                                                        \
  }


namespace mitk {

class BaseData;

/**
 * @brief Superclass of all classes generating some kind of mitk::BaseData.
 *
 * Superclass of all classes generating some kind of mitk::BaseData.
 * In itk and vtk the generated result of a ProcessObject is only guaranteed
 * to be up-to-date, when Update() of the ProcessObject or the generated
 * DataObject is called immediately before access of the data stored in the
 * DataObject. This is also true for subclasses of mitk::BaseProcess. But
 * many of the subclasses of mitk::BaseProcess define additional access
 * functions to the generated output that guarantee an up-to-date result, see
 * for example mitk::ImageSource.
 * @ingroup Process
 */
class MITKCORE_EXPORT BaseDataSource : public itk::ProcessObject
{
public:

  mitkClassMacroItkParent(BaseDataSource, itk::ProcessObject)

  typedef BaseData OutputType;
  typedef itk::DataObject::DataObjectIdentifierType DataObjectIdentifierType;

  mitkBaseDataSourceGetOutputDeclarations

  /** @brief Graft the specified BaseData onto this BaseDataSource's output.
   *
   * This method is used when a
   * process object is implemented using a mini-pipeline which is
   * defined in its GenerateData() method.  The usage is:
   *
   * \code
   *    // setup the mini-pipeline to process the input to this filter
   *    firstFilterInMiniPipeline->SetInput( this->GetInput() );

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
   */
  virtual void GraftOutput(OutputType* output);

  /**
   * Graft the specified base data object onto this BaseDataSource's named
   * output. This is similar to the GraftOutput method except it
   * allows you to specify which output is affected.
   * See the GraftOutput for general usage information.
   */
  virtual void GraftOutput(const DataObjectIdentifierType & key, OutputType* output);

  /** @brief Graft the specified base data object onto this BaseDataSource's idx'th
   * output.
   *
   * This is the similar to GraftOutput method except is
   * allows you specify which output is affected. The specified index
   * must be a valid output number (less than
   * ProcessObject::GetNumberOfOutputs()). See the GraftOutput for
   * general usage information.
   */
  virtual void GraftNthOutput(unsigned int idx, OutputType* output);

  /**
   * @sa itk::ProcessObject::MakeOutput(DataObjectPointerArraySizeType)
   */
  virtual DataObjectPointer MakeOutput ( DataObjectPointerArraySizeType idx ) override = 0;

  /**
   * @sa itk::ProcessObject::MakeOutput(const DataObjectIdentifierType&)
   */
  virtual DataObjectPointer MakeOutput(const DataObjectIdentifierType& name) override = 0;

  /**
   * @brief Access itk::ProcessObject::m_Updating
   *
   * m_Updating indicates when the pipeline is executing.
   * It prevents infinite recursion when pipelines have loops.
   * \sa itk::ProcessObject::m_Updating
   **/
  bool Updating() const;

protected:

  BaseDataSource();
  virtual ~BaseDataSource();

  // purposely not implemented
  static Pointer New();

};

} // namespace mitk

#endif /* MITK_BASE_DATA_SOURCE_H */
