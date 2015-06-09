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

#ifndef _MITKVTKSHADERPROGRAM_H_
#define _MITKVTKSHADERPROGRAM_H_

#include <mitkIShaderRepository.h>

#include <vtkShaderProgram2.h>
#include <vtkSmartPointer.h>

namespace mitk {

/**
 * \brief SHADERTODO
 */
class VtkShaderProgram : public IShaderRepository::ShaderProgram
{
public:

  mitkClassMacro( VtkShaderProgram, IShaderRepository::ShaderProgram )
  itkFactorylessNewMacro( Self )

  /**
   * Constructor
   */
  VtkShaderProgram();

  /**
   * Destructor
   */
  virtual ~VtkShaderProgram();

  virtual void Activate() override;
  virtual void Deactivate() override;

  void SetVtkShaderProgram(vtkSmartPointer<vtkShaderProgram2> p);
  vtkSmartPointer<vtkShaderProgram2> GetVtkShaderProgram() const;
  itk::TimeStamp& GetShaderTimestampUpdate();

private:

  vtkSmartPointer<vtkShaderProgram2> m_VtkShaderProgram;
  itk::TimeStamp m_ShaderTimestampUpdate;

};

} //end of namespace mitk
#endif
