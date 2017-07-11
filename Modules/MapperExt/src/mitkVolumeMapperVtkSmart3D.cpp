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
#include "mitkTransferFunctionProperty.h"
#include "mitkTransferFunctionInitializer.h"
#include <vtkObjectFactory.h>
#include <vtkRenderingOpenGL2ObjectFactory.h>
#include <vtkRenderingVolumeOpenGL2ObjectFactory.h>
#include <vtkColorTransferFunction.h>
#include <vtkPiecewiseFunction.h>

void mitk::VolumeMapperVtkSmart3D::GenerateDataForRenderer(mitk::BaseRenderer *renderer)
{
  bool value;
  this->GetDataNode()->GetBoolProperty("volumerendering", value, renderer);
  if (!value)
  {
    m_Volume->VisibilityOff();
    return;
  }
  else
  {
    m_Volume->VisibilityOn();
  }

  UpdateTransferFunctions(renderer);
}

vtkProp* mitk::VolumeMapperVtkSmart3D::GetVtkProp(mitk::BaseRenderer *renderer)
{
  if (!m_Volume->GetMapper())
  {
    createMapper(GetInputImage());
    createVolume();
    createVolumeProperty();
  }

  return m_Volume;
}

void mitk::VolumeMapperVtkSmart3D::ApplyProperties(vtkActor *actor, mitk::BaseRenderer *renderer)
{

}

void mitk::VolumeMapperVtkSmart3D::SetDefaultProperties(mitk::DataNode *node, mitk::BaseRenderer *renderer, bool overwrite)
{
  node->AddProperty("volumerendering", mitk::BoolProperty::New(false), renderer, overwrite);
}

vtkImageData* mitk::VolumeMapperVtkSmart3D::GetInputImage()
{
  mitk::Image *input = const_cast<mitk::Image *>(static_cast<const mitk::Image *>(this->GetDataNode()->GetData()));
  vtkImageData* img = input->GetVtkImageData(this->GetTimestep());
  img->SetSpacing(1,1,1);

  return img;
}

void mitk::VolumeMapperVtkSmart3D::createMapper(vtkImageData* imageData)
{
  m_SmartVolumeMapper->SetBlendModeToComposite();
  m_SmartVolumeMapper->SetInputData(imageData);
}

void mitk::VolumeMapperVtkSmart3D::createVolume()
{    
  m_Volume->VisibilityOff();
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
  compositeOpacity->AddPoint(100.0, 0.0);
  compositeOpacity->AddPoint(3000.0, 1.0);
  m_VolumeProperty->SetScalarOpacity(compositeOpacity); // composite first.

  vtkSmartPointer<vtkColorTransferFunction> color =
    vtkSmartPointer<vtkColorTransferFunction>::New();
  color->AddRGBPoint(0.0, 1.0, 1.0, 1.0);
  color->AddRGBPoint(255.0, 1.0, 1.0, 1.0);
  m_VolumeProperty->SetColor(color);
}

void mitk::VolumeMapperVtkSmart3D::UpdateTransferFunctions(mitk::BaseRenderer *renderer)
{
  vtkSmartPointer<vtkPiecewiseFunction> opacityTransferFunction;
  vtkSmartPointer<vtkPiecewiseFunction> gradientTransferFunction;
  vtkSmartPointer<vtkColorTransferFunction> colorTransferFunction;

  bool isBinary = false;

  this->GetDataNode()->GetBoolProperty("binary", isBinary, renderer);

  if (isBinary)
  {
    colorTransferFunction = vtkSmartPointer<vtkColorTransferFunction>::New();

    float rgb[3];
    if (!GetDataNode()->GetColor(rgb, renderer))
      rgb[0] = rgb[1] = rgb[2] = 1;
    colorTransferFunction->AddRGBPoint(0, rgb[0], rgb[1], rgb[2]);
    colorTransferFunction->Modified();
  }
  else
  {
    mitk::TransferFunctionProperty *transferFunctionProp =
      dynamic_cast<mitk::TransferFunctionProperty *>(this->GetDataNode()->GetProperty("TransferFunction", renderer));

    if (transferFunctionProp)
    {
      opacityTransferFunction = transferFunctionProp->GetValue()->GetScalarOpacityFunction();
      gradientTransferFunction = transferFunctionProp->GetValue()->GetGradientOpacityFunction();
      colorTransferFunction = transferFunctionProp->GetValue()->GetColorTransferFunction();
    }
    else
    {
      opacityTransferFunction = vtkSmartPointer<vtkPiecewiseFunction>::New();
      gradientTransferFunction = vtkSmartPointer<vtkPiecewiseFunction>::New();
      colorTransferFunction = vtkSmartPointer<vtkColorTransferFunction>::New();
    }
  }

  m_VolumeProperty->SetColor(colorTransferFunction);
  m_VolumeProperty->SetScalarOpacity(opacityTransferFunction);
  m_VolumeProperty->SetGradientOpacity(gradientTransferFunction);

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


