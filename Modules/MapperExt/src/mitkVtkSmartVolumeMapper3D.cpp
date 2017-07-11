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

#include "mitkVtkSmartVolumeMapper3D.h"

void mitk::VtkSmartVolumeMapper3D::GenerateDataForRenderer(mitk::BaseRenderer *renderer)
{

}

vtkProp* mitk::VtkSmartVolumeMapper3D::GetVtkProp(mitk::BaseRenderer *renderer)
{
  if (!m_Volume)
  {
    createVolume();
  }

  return m_Volume;
}

void mitk::VtkSmartVolumeMapper3D::ApplyProperties(vtkActor *actor, mitk::BaseRenderer *renderer)
{

}

void mitk::VtkSmartVolumeMapper3D::SetDefaultProperties(mitk::DataNode *node, mitk::BaseRenderer *renderer, bool overwrite)
{

}

void mitk::VtkSmartVolumeMapper3D::MitkRenderVolumetricGeometry(mitk::BaseRenderer *renderer)
{

}

vtkImageData* mitk::VtkSmartVolumeMapper3D::GetInputImage()
{
  mitk::Image *input = const_cast<mitk::Image *>(static_cast<const mitk::Image *>(this->GetDataNode()->GetData()));
  return input->GetVtkImageData(this->GetTimestep());
}

void mitk::VtkSmartVolumeMapper3D::createMapper(vtkImageData* imageData)
{
  vtkSmartPointer<vtkSmartVolumeMapper> m_Mapper =
    vtkSmartPointer<vtkSmartVolumeMapper>::New();
  m_Mapper->SetBlendModeToComposite(); // composite first
  m_Mapper->SetInputData(imageData);
}

void mitk::VtkSmartVolumeMapper3D::createVolume()
{
  if (!m_Mapper)
  {
    createMapper(GetInputImage());
  }
  if (!m_VolumeProperty)
  {
    createVolumeProperty();
  }

  m_Volume = vtkSmartPointer<vtkVolume>::New();
  m_Volume->VisibilityOff();
  m_Volume->SetMapper(m_Mapper);
  m_Volume->SetProperty(m_VolumeProperty);
}

void mitk::VtkSmartVolumeMapper3D::createVolumeProperty()
{
  m_VolumeProperty = vtkSmartPointer<vtkVolumeProperty>::New();
  m_VolumeProperty->ShadeOff();
  m_VolumeProperty->SetInterpolationType(VTK_LINEAR_INTERPOLATION);
}

mitk::VtkSmartVolumeMapper3D::VtkSmartVolumeMapper3D()
{
 
}

mitk::VtkSmartVolumeMapper3D::~VtkSmartVolumeMapper3D()
{

}


