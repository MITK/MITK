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

#ifndef _MITK_DATA_TREE_NODE_SOURCE_H
#define _MITK_DATA_TREE_NODE_SOURCE_H

#include "itkProcessObject.h"
#include "mitkDataNode.h"

namespace mitk
{
  /**
   * @brief Superclass of all classes generating data tree nodes (instances of class
   * mitk::DataNode) as output.
   *
   * In itk and vtk the generated result of a ProcessObject is only guaranteed
   * to be up-to-date, when Update() of the ProcessObject or the generated
   * DataObject is called immediately before access of the data stored in the
   * DataObject. This is also true for subclasses of mitk::BaseProcess and thus
   * for mitk::DataNodeSource.
   * @ingroup Process
   */
  class MITKCORE_EXPORT DataNodeSource : public itk::ProcessObject
  {
  public:
    mitkClassMacroItkParent(DataNodeSource, itk::ProcessObject)

      itkFactorylessNewMacro(Self) itkCloneMacro(Self)

        typedef mitk::DataNode OutputType;

    typedef OutputType::Pointer OutputTypePointer;

    /**
     * Allocates a new output object and returns it. Currently the
     * index idx is not evaluated.
     * @param idx the index of the output for which an object should be created
     * @returns the new object
     */
    DataObjectPointer MakeOutput(DataObjectPointerArraySizeType idx) override;

    /**
     * This is a default implementation to make sure we have something.
     * Once all the subclasses of ProcessObject provide an appopriate
     * MakeOutput(), then ProcessObject::MakeOutput() can be made pure
     * virtual.
     */
    DataObjectPointer MakeOutput(const DataObjectIdentifierType &name) override;

    OutputType *GetOutput();
    const OutputType *GetOutput() const;
    OutputType *GetOutput(DataObjectPointerArraySizeType idx);
    const OutputType *GetOutput(DataObjectPointerArraySizeType idx) const;

  protected:
    DataNodeSource();

    ~DataNodeSource() override;
  };
}
#endif // #define _MITK_BASE_DATA_SOURCE_H
