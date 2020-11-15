/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkVolumeMapperVtkSmart3D.h"
#include "mitkTransferFunctionProperty.h"
#include "mitkTransferFunctionInitializer.h"
#include "mitkLevelWindowProperty.h"
#include <vtkObjectFactory.h>
#include <vtkColorTransferFunction.h>
#include <vtkPiecewiseFunction.h>
#include <vtkAutoInit.h>

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
  UpdateRenderMode(renderer);
  this->Modified();
}

vtkProp* mitk::VolumeMapperVtkSmart3D::GetVtkProp(mitk::BaseRenderer *)
{
  if (!m_Volume->GetMapper())
  {
    createMapper(GetInputImage());
    createVolume();
    createVolumeProperty();
  }

  return m_Volume;
}

void mitk::VolumeMapperVtkSmart3D::ApplyProperties(vtkActor *, mitk::BaseRenderer *)
{

}

void mitk::VolumeMapperVtkSmart3D::SetDefaultProperties(mitk::DataNode *node, mitk::BaseRenderer *renderer, bool overwrite)
{
  // GPU_INFO << "SetDefaultProperties";

  node->AddProperty("volumerendering", mitk::BoolProperty::New(false), renderer, overwrite);
  node->AddProperty("volumerendering.usemip", mitk::BoolProperty::New(false), renderer, overwrite);

  node->AddProperty("volumerendering.cpu.ambient", mitk::FloatProperty::New(0.10f), renderer, overwrite);
  node->AddProperty("volumerendering.cpu.diffuse", mitk::FloatProperty::New(0.50f), renderer, overwrite);
  node->AddProperty("volumerendering.cpu.specular", mitk::FloatProperty::New(0.40f), renderer, overwrite);
  node->AddProperty("volumerendering.cpu.specular.power", mitk::FloatProperty::New(16.0f), renderer, overwrite);
  node->AddProperty("volumerendering.usegpu", mitk::BoolProperty::New(false), renderer, overwrite);
  node->AddProperty("volumerendering.useray", mitk::BoolProperty::New(false), renderer, overwrite);

  node->AddProperty("volumerendering.gpu.ambient", mitk::FloatProperty::New(0.25f), renderer, overwrite);
  node->AddProperty("volumerendering.gpu.diffuse", mitk::FloatProperty::New(0.50f), renderer, overwrite);
  node->AddProperty("volumerendering.gpu.specular", mitk::FloatProperty::New(0.40f), renderer, overwrite);
  node->AddProperty("volumerendering.gpu.specular.power", mitk::FloatProperty::New(16.0f), renderer, overwrite);

  node->AddProperty("binary", mitk::BoolProperty::New(false), renderer, overwrite);

  mitk::Image::Pointer image = dynamic_cast<mitk::Image *>(node->GetData());
  if (image.IsNotNull() && image->IsInitialized())
  {
    if ((overwrite) || (node->GetProperty("TransferFunction", renderer) == nullptr))
    {
      // add a default transfer function
      mitk::TransferFunction::Pointer tf = mitk::TransferFunction::New();
      mitk::TransferFunctionInitializer::Pointer tfInit = mitk::TransferFunctionInitializer::New(tf);
      tfInit->SetTransferFunctionMode(0);
      node->SetProperty("TransferFunction", mitk::TransferFunctionProperty::New(tf.GetPointer()));
    }
  }

  Superclass::SetDefaultProperties(node, renderer, overwrite);
}

vtkImageData* mitk::VolumeMapperVtkSmart3D::GetInputImage()
{
  auto input = dynamic_cast<mitk::Image*>(this->GetDataNode()->GetData());
  return input->GetVtkImageData(this->GetTimestep());
}

void mitk::VolumeMapperVtkSmart3D::createMapper(vtkImageData* imageData)
{
  Vector3D spacing;
  FillVector3D(spacing, 1.0, 1.0, 1.0);

  m_ImageChangeInformation->SetInputData(imageData);
  m_ImageChangeInformation->SetOutputSpacing(spacing.GetDataPointer());

  m_SmartVolumeMapper->SetBlendModeToComposite();
  m_SmartVolumeMapper->SetInputConnection(m_ImageChangeInformation->GetOutputPort());
}

void mitk::VolumeMapperVtkSmart3D::createVolume()
{
  m_Volume->VisibilityOff();
  m_Volume->SetMapper(m_SmartVolumeMapper);
  m_Volume->SetProperty(m_VolumeProperty);
}

void mitk::VolumeMapperVtkSmart3D::createVolumeProperty()
{
  m_VolumeProperty->ShadeOn();
  m_VolumeProperty->SetInterpolationType(VTK_LINEAR_INTERPOLATION);
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

    opacityTransferFunction = vtkSmartPointer<vtkPiecewiseFunction>::New();
    gradientTransferFunction = vtkSmartPointer<vtkPiecewiseFunction>::New();
  }
  else
  {
    auto *transferFunctionProp =
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


void mitk::VolumeMapperVtkSmart3D::UpdateRenderMode(mitk::BaseRenderer *renderer)
{
  bool usegpu = false;
  bool useray = false;
  bool usemip = false;
  this->GetDataNode()->GetBoolProperty("volumerendering.usegpu", usegpu);
  this->GetDataNode()->GetBoolProperty("volumerendering.useray", useray);
  this->GetDataNode()->GetBoolProperty("volumerendering.usemip", usemip);

  if (usegpu)
    m_SmartVolumeMapper->SetRequestedRenderModeToGPU();
  else if (useray)
    m_SmartVolumeMapper->SetRequestedRenderModeToRayCast();
  else
    m_SmartVolumeMapper->SetRequestedRenderModeToDefault();

  int blendMode;
  if (this->GetDataNode()->GetIntProperty("volumerendering.blendmode", blendMode))
    m_SmartVolumeMapper->SetBlendMode(blendMode);
  else if (usemip)
    m_SmartVolumeMapper->SetBlendMode(vtkSmartVolumeMapper::MAXIMUM_INTENSITY_BLEND);

  // shading parameter
  if (m_SmartVolumeMapper->GetRequestedRenderMode() == vtkSmartVolumeMapper::GPURenderMode)
  {
    float value = 0;
    if (this->GetDataNode()->GetFloatProperty("volumerendering.gpu.ambient", value, renderer))
      m_VolumeProperty->SetAmbient(value);
    if (this->GetDataNode()->GetFloatProperty("volumerendering.gpu.diffuse", value, renderer))
      m_VolumeProperty->SetDiffuse(value);
    if (this->GetDataNode()->GetFloatProperty("volumerendering.gpu.specular", value, renderer))
      m_VolumeProperty->SetSpecular(value);
    if (this->GetDataNode()->GetFloatProperty("volumerendering.gpu.specular.power", value, renderer))
      m_VolumeProperty->SetSpecularPower(value);
  }
  else
  {
    float value = 0;
    if (this->GetDataNode()->GetFloatProperty("volumerendering.cpu.ambient", value, renderer))
      m_VolumeProperty->SetAmbient(value);
    if (this->GetDataNode()->GetFloatProperty("volumerendering.cpu.diffuse", value, renderer))
      m_VolumeProperty->SetDiffuse(value);
    if (this->GetDataNode()->GetFloatProperty("volumerendering.cpu.specular", value, renderer))
      m_VolumeProperty->SetSpecular(value);
    if (this->GetDataNode()->GetFloatProperty("volumerendering.cpu.specular.power", value, renderer))
      m_VolumeProperty->SetSpecularPower(value);
  }
}

mitk::VolumeMapperVtkSmart3D::VolumeMapperVtkSmart3D()
{
  m_SmartVolumeMapper = vtkSmartPointer<vtkSmartVolumeMapper>::New();
  m_SmartVolumeMapper->SetBlendModeToComposite();
  m_ImageChangeInformation = vtkSmartPointer<vtkImageChangeInformation>::New();
  m_VolumeProperty = vtkSmartPointer<vtkVolumeProperty>::New();
  m_Volume = vtkSmartPointer<vtkVolume>::New();
}

mitk::VolumeMapperVtkSmart3D::~VolumeMapperVtkSmart3D()
{

}


