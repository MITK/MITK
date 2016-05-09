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

#include "mitkVtkShaderProgram.h"

#include "vtkShader.h"

mitk::VtkShaderProgram::VtkShaderProgram() : m_VtkShaderProgram(NULL)
{
}

mitk::VtkShaderProgram::~VtkShaderProgram()
{
}

void mitk::VtkShaderProgram::Activate()
{
  if (m_VtkShaderProgram)
  {
//    m_VtkShaderProgram->Use(); //TODO 18922
  }
}

void mitk::VtkShaderProgram::Deactivate()
{
  if (m_VtkShaderProgram)
  {
//    m_VtkShaderProgram->Restore(); //TODO 18922
  }
}

void mitk::VtkShaderProgram::SetVtkShaderProgram(vtkSmartPointer<vtkShaderProgram> p)
{
  m_VtkShaderProgram = p;
}

vtkSmartPointer<vtkShaderProgram> mitk::VtkShaderProgram::GetVtkShaderProgram() const
{
  return m_VtkShaderProgram;
}

itk::TimeStamp &mitk::VtkShaderProgram::GetShaderTimestampUpdate()
{
  return m_ShaderTimestampUpdate;
}
