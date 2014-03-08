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
#include "vtkShader2.h"

mitk::VtkShaderProgram::VtkShaderProgram()
{
}

mitk::VtkShaderProgram::~VtkShaderProgram()
{
}

void mitk::VtkShaderProgram::SetVtkShaderProgram(vtkShaderProgram2* p) { m_VtkShaderProgram = p; }

vtkShaderProgram2*mitk::VtkShaderProgram::GetVtkShaderProgram() const { return m_VtkShaderProgram; }

itk::TimeStamp&mitk::VtkShaderProgram::GetShaderTimestampUpdate(){return m_ShaderTimestampUpdate;}
