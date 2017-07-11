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
#include <vtkObjectFactory.h>
#include <vtkRenderingOpenGL2ObjectFactory.h>
#include <vtkRenderingVolumeOpenGL2ObjectFactory.h>
#include <vtkColorTransferFunction.h>
#include <vtkPiecewiseFunction.h>

void mitk::VolumeMapperVtkSmart3D::GenerateDataForRenderer(mitk::BaseRenderer *renderer)
{
  createMapper(GetInputImage());
  createVolume();
  createVolumeProperty();
  m_SmartVolumeMapper->Update();
  m_Volume->Update();
  MITK_INFO << "rendering ...";
}

vtkProp* mitk::VolumeMapperVtkSmart3D::GetVtkProp(mitk::BaseRenderer *renderer)
{
  return m_Volume;
}

void mitk::VolumeMapperVtkSmart3D::ApplyProperties(vtkActor *actor, mitk::BaseRenderer *renderer)
{

}

void mitk::VolumeMapperVtkSmart3D::SetDefaultProperties(mitk::DataNode *node, mitk::BaseRenderer *renderer, bool overwrite)
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
  m_Volume->VisibilityOn();
  m_Volume->SetMapper(m_SmartVolumeMapper);
  m_Volume->SetProperty(m_VolumeProperty);
}

void mitk::VolumeMapperVtkSmart3D::createVolumeProperty()
{
  m_VolumeProperty->ShadeOff();
  m_VolumeProperty->SetInterpolationType(VTK_LINEAR_INTERPOLATION);

  vtkSmartPointer<vtkPiecewiseFunction> compositeOpacity =
    vtkSmartPointer<vtkPiecewiseFunction>::New();
  compositeOpacity->AddPoint(-1000.0, 0.0);
  compositeOpacity->AddPoint(400.0, 1.0);
  m_VolumeProperty->SetScalarOpacity(compositeOpacity); // composite first.

  vtkSmartPointer<vtkColorTransferFunction> color =
    vtkSmartPointer<vtkColorTransferFunction>::New();
  color->AddRGBPoint(0.0, 1.0, 1.0, 1.0);
  color->AddRGBPoint(255.0, 1.0, 1.0, 1.0);
  m_VolumeProperty->SetColor(color);
}

mitk::VolumeMapperVtkSmart3D::VolumeMapperVtkSmart3D()
{
  vtkObjectFactory::RegisterFactory(vtkRenderingOpenGL2ObjectFactory::New());
  vtkObjectFactory::RegisterFactory(vtkRenderingVolumeOpenGL2ObjectFactory::New());

  m_SmartVolumeMapper = vtkSmartPointer<vtkSmartVolumeMapper>::New();
  m_SmartVolumeMapper->SetBlendModeToComposite();
  m_VolumeProperty = vtkSmartPointer<vtkVolumeProperty>::New();
  m_Volume = vtkSmartPointer<vtkVolume>::New();
}

mitk::VolumeMapperVtkSmart3D::~VolumeMapperVtkSmart3D()
{

}


