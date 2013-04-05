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


#ifndef BASEPROCESS_H_HEADER_INCLUDED_C19BE6FC
#define BASEPROCESS_H_HEADER_INCLUDED_C19BE6FC

#if(_MSC_VER==1200)
#include <itkFixedArray.h>
#endif

#include <MitkExports.h>
#include <mitkCommon.h>
#include <itkProcessObject.h>

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
class MITK_CORE_EXPORT BaseProcess : public itk::ProcessObject
{
public:

  mitkClassMacro(BaseProcess, itk::ProcessObject)

  /**
   * @brief Access itk::ProcessObject::m_Updating
   *
   * m_Updating indicates when the pipeline is executing.
   * It prevents infinite recursion when pipelines have loops.
   * \sa itk::ProcessObject::m_Updating
   **/
  bool Updating() const
  {
    return m_Updating;
  }

protected:

  BaseProcess();

  virtual ~BaseProcess();

};

} // namespace mitk

#endif /* BASEPROCESS_H_HEADER_INCLUDED_C19BE6FC */
