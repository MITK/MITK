/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkDataNodeSource_h
#define mitkDataNodeSource_h

#include <itkProcessObject.h>
#include <mitkDataNode.h>

namespace mitk
{
  /**
   * \brief Superclass of all classes generating DataNode instances as output.
   *
   * In ITK and VTK the generated result of a ProcessObject is only guaranteed
   * to be up-to-date when Update() of the ProcessObject or the generated
   * DataObject is called immediately before accessing the stored data.
   * This is also true for subclasses of mitk::DataNodeSource.
   *
   * \ingroup Process
   * \sa DataNode
   */
  class MITKCORE_EXPORT DataNodeSource : public itk::ProcessObject
  {
  public:
    mitkClassMacroItkParent(DataNodeSource, itk::ProcessObject);

      itkFactorylessNewMacro(Self);

      itkCloneMacro(Self);

      /** \brief The output data type (mitk::DataNode). */
      typedef mitk::DataNode OutputType;

    /** \brief Pointer type for the output. */
    typedef OutputType::Pointer OutputTypePointer;

    /**
     * \brief Allocate and return a new output object.
     *
     * Currently the index is not evaluated; a new DataNode is always created.
     *
     * \param idx The index of the output for which an object should be created.
     * \return A smart pointer to the newly created DataNode.
     */
    DataObjectPointer MakeOutput(DataObjectPointerArraySizeType idx) override;

    /**
     * \brief Allocate and return a new output object by name.
     *
     * Default implementation that creates a new DataNode. If the name
     * corresponds to an indexed output, the indexed version is called.
     *
     * \param name The identifier of the output.
     * \return A smart pointer to the newly created DataNode.
     */
    DataObjectPointer MakeOutput(const DataObjectIdentifierType &name) override;

    /**
     * \brief Get the primary output DataNode.
     * \return Pointer to the primary output.
     */
    OutputType *GetOutput();

    /** \overload */
    const OutputType *GetOutput() const;

    /**
     * \brief Get the output DataNode at the given index.
     * \param idx The output index.
     * \return Pointer to the requested output.
     */
    OutputType *GetOutput(DataObjectPointerArraySizeType idx);

    /** \overload */
    const OutputType *GetOutput(DataObjectPointerArraySizeType idx) const;

  protected:
    /** \brief Constructor. Creates the initial output. */
    DataNodeSource();

    /** \brief Destructor. */
    ~DataNodeSource() override;
  };
}
#endif
