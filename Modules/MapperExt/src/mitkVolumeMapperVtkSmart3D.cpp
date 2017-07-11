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

#include "mitkVolumeMapperVtkSmart3D.h"

void mitk::VolumeMapperVtkSmart3D::GenerateDataForRenderer(mitk::BaseRenderer *renderer)
{
  m_SmartVolumeMapper = vtkSmartPointer<vtkSmartVolumeMapper>::New();
}

vtkProp* mitk::VolumeMapperVtkSmart3D::GetVtkProp(mitk::BaseRenderer *renderer)
{
  if (!m_Volume->GetMapper())
  {
    createVolume();
  }

  return m_Volume;
}

void mitk::VolumeMapperVtkSmart3D::ApplyProperties(vtkActor *actor, mitk::BaseRenderer *renderer)
{

}

void mitk::VolumeMapperVtkSmart3D::SetDefaultProperties(mitk::DataNode *node, mitk::BaseRenderer *renderer, bool overwrite)
{

}

void mitk::VolumeMapperVtkSmart3D::MitkRenderVolumetricGeometry(mitk::BaseRenderer *renderer)
{

}

vtkImageData* mitk::VolumeMapperVtkSmart3D::GetInputImage()
{
  mitk::Image *input = const_cast<mitk::Image *>(static_cast<const mitk::Image *>(this->GetDataNode()->GetData()));
  return input->GetVtkImageData(this->GetTimestep());
}

void mitk::VolumeMapperVtkSmart3D::createMapper(vtkImageData* imageData)
{
  m_SmartVolumeMapper->SetBlendModeToComposite(); // composite first
  m_SmartVolumeMapper->SetInputData(imageData);
}

void mitk::VolumeMapperVtkSmart3D::createVolume()
{
  if (!m_SmartVolumeMapper->GetInput())
  {
    createMapper(GetInputImage());
  }

  if (1)
    createVolumeProperty();

  m_Volume->VisibilityOff();
  m_Volume->SetMapper(m_SmartVolumeMapper);
  m_Volume->SetProperty(m_VolumeProperty);
}

void mitk::VolumeMapperVtkSmart3D::createVolumeProperty()
{
  m_VolumeProperty->ShadeOff();
  m_VolumeProperty->SetInterpolationType(VTK_LINEAR_INTERPOLATION);
}

mitk::VolumeMapperVtkSmart3D::VolumeMapperVtkSmart3D()
{
  m_VolumeProperty = vtkSmartPointer<vtkVolumeProperty>::New();
  m_Volume = vtkSmartPointer<vtkVolume>::New();
}

mitk::VolumeMapperVtkSmart3D::~VolumeMapperVtkSmart3D()
{

}


