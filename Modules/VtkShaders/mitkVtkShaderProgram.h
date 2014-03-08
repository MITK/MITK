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
#include <VtkShadersExports.h>
#include <vtkShaderProgram2.h>
#include <vtkSmartPointer.h>

namespace mitk {

/**
 * \brief SHADERTODO
 */
class VtkShaders_EXPORT VtkShaderProgram : public IShaderRepository::ShaderProgram
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

  virtual void Activate()
  {
    if(m_VtkShaderProgram)
    {
      m_VtkShaderProgram->UseProgram();

    }
  }
  virtual void Deactivate()
  {
    if(m_VtkShaderProgram)
    {
      m_VtkShaderProgram->UnuseProgram();
    }
  }

  void SetVtkShaderProgram( vtkShaderProgram2 * p );
  vtkShaderProgram2 * GetVtkShaderProgram() const;
  itk::TimeStamp& GetShaderTimestampUpdate();

private:

  vtkShaderProgram2* m_VtkShaderProgram;
  itk::TimeStamp m_ShaderTimestampUpdate;

};

} //end of namespace mitk
#endif
