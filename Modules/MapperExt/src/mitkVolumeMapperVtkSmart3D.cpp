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
#include "mitkLevelWindowProperty.h"
#include <mitkImageVtkAccessor.h>
#include <vtkObjectFactory.h>
#include <vtkRenderingOpenGL2ObjectFactory.h>
#include <vtkRenderingVolumeOpenGL2ObjectFactory.h>
#include <vtkColorTransferFunction.h>
#include <vtkPiecewiseFunction.h>

void mitk::VolumeMapperVtkSmart3D::initialize()
{
  if (!m_Volume->GetMapper() || this->GetTimestep() != m_TimeStep) {
    createMapper(GetInputImage());
    createVolume();
    createVolumeProperty();
  }
}

void mitk::VolumeMapperVtkSmart3D::GenerateDataForRenderer(mitk::BaseRenderer *renderer)
{
  initialize();

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

vtkProp* mitk::VolumeMapperVtkSmart3D::GetVtkProp(mitk::BaseRenderer *renderer)
{
  initialize();
  return m_Volume;
}

void mitk::VolumeMapperVtkSmart3D::ApplyProperties(vtkActor *actor, mitk::BaseRenderer *renderer)
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
    if ((overwrite) || (node->GetProperty("levelwindow", renderer) == nullptr))
    {
      mitk::LevelWindowProperty::Pointer levWinProp = mitk::LevelWindowProperty::New();
      mitk::LevelWindow levelwindow;
      levelwindow.SetAuto(image);
      levWinProp->SetLevelWindow(levelwindow);
      node->SetProperty("levelwindow", levWinProp, renderer);
    }

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

void mitk::VolumeMapperVtkSmart3D::setClippingPlanes(vtkPlanes* planes)
{
  initialize();
  m_SmartVolumeMapper->SetClippingPlanes(planes);
}

vtkImageData* mitk::VolumeMapperVtkSmart3D::GetInputImage()
{
  mitk::Image *input = const_cast<mitk::Image *>(static_cast<const mitk::Image *>(this->GetDataNode()->GetData()));
  m_TimeStep = this->GetTimestep();
  int maxTime = input->GetTimeSteps() - 1;
  mitk::ImageVtkAccessor accessor(input);
  vtkImageData* img = accessor.getVtkImageData(m_TimeStep < maxTime ? m_TimeStep : maxTime);

  if (img) {
    img->SetSpacing(1,1,1);
  }

  return img;
}

void mitk::VolumeMapperVtkSmart3D::createMapper(vtkImageData* imageData)
{
  m_SmartVolumeMapper = vtkSmartPointer<vtkSmartVolumeMapper>::New();
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

    opacityTransferFunction = m_BinaryOpacityTransferFunction;
    gradientTransferFunction = m_BinaryGradientTransferFunction;
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

mitk::VolumeMapperVtkSmart3D::VolumeMapperVtkSmart3D() :
  m_TimeStep(0)
{
  vtkObjectFactory::RegisterFactory(vtkRenderingOpenGL2ObjectFactory::New());
  vtkObjectFactory::RegisterFactory(vtkRenderingVolumeOpenGL2ObjectFactory::New());

  m_VolumeProperty = vtkSmartPointer<vtkVolumeProperty>::New();
  m_Volume = vtkSmartPointer<vtkVolume>::New();

  m_BinaryOpacityTransferFunction = vtkSmartPointer<vtkPiecewiseFunction>::New();
  m_BinaryOpacityTransferFunction->AddPoint(0, 0.0);
  m_BinaryOpacityTransferFunction->AddPoint(1, 1.0);

  m_BinaryGradientTransferFunction = vtkSmartPointer<vtkPiecewiseFunction>::New();
  m_BinaryGradientTransferFunction->AddPoint(0.0, 1.0);
}

mitk::VolumeMapperVtkSmart3D::~VolumeMapperVtkSmart3D()
{

}

