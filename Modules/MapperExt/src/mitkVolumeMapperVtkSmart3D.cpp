/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkVolumeMapperVtkSmart3D.h>
#include <mitkTransferFunctionProperty.h>
#include <mitkTransferFunctionInitializer.h>
#include <mitkLevelWindowProperty.h>
#include <vtkObjectFactory.h>
#include <vtkColorTransferFunction.h>
#include <vtkPiecewiseFunction.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkAutoInit.h>

namespace
{
  /** Whether this frame is one of an ongoing interaction, judged the way
   * vtkSmartVolumeMapper judges it. Absent a render window there is nothing to
   * ask, and a still frame is the safe answer: it costs time, not fidelity.
   */
  bool IsInteractiveRender(mitk::BaseRenderer *renderer, double interactiveUpdateRate)
  {
    auto *vtkRenderer = renderer->GetVtkRenderer();

    if (nullptr == vtkRenderer)
      return false;

    auto *renderWindow = vtkRenderer->GetRenderWindow();

    return nullptr != renderWindow && renderWindow->GetDesiredUpdateRate() >= interactiveUpdateRate;
  }
}

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
    createMapper(GetInputImage());
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

void mitk::VolumeMapperVtkSmart3D::UpdateVtkTransform(mitk::BaseRenderer *renderer)
{
  auto *imageData = this->GetInputImage();

  if (nullptr == imageData)
  {
    Superclass::UpdateVtkTransform(renderer);
    return;
  }

  // Read the spacing back from the image the mapper was handed rather than from
  // the geometry, so the two halves of the placement cannot disagree if the
  // geometry is rescaled after the image data was built.
  double spacing[3];
  imageData->GetSpacing(spacing);

  // IndexToWorld carries the spacing the image itself now supplies. Applying
  // both would size the volume by it twice.
  m_DataToWorld->SetMatrix(this->GetDataNode()->GetVtkTransform(this->GetTimestep())->GetMatrix());
  m_DataToWorld->Scale(1.0 / spacing[0], 1.0 / spacing[1], 1.0 / spacing[2]);

  m_Volume->SetUserTransform(m_DataToWorld);
}

void mitk::VolumeMapperVtkSmart3D::SetDefaultProperties(mitk::DataNode *node, mitk::BaseRenderer *renderer, bool overwrite)
{
  // GPU_INFO << "SetDefaultProperties";

  const auto *image = dynamic_cast<const mitk::Image *>(node->GetData());

  if (image == nullptr || !image->IsInitialized())
    return;

  node->AddProperty("volumerendering", mitk::BoolProperty::New(false), renderer, overwrite);

  node->AddProperty("volumerendering.ambient", mitk::FloatProperty::New(0.1f), renderer, overwrite);
  node->AddProperty("volumerendering.diffuse", mitk::FloatProperty::New(0.50f), renderer, overwrite);
  node->AddProperty("volumerendering.specular", mitk::FloatProperty::New(0.40f), renderer, overwrite);
  node->AddProperty("volumerendering.specular.power", mitk::FloatProperty::New(16.0f), renderer, overwrite);
  node->AddProperty("volumerendering.shade", mitk::BoolProperty::New(true), renderer, overwrite);
  node->AddProperty("volumerendering.scattering.blend", mitk::FloatProperty::New(0.0f), renderer, overwrite);
  // Reach bounds the secondary rays and is the whole cost of scattering; 0.0
  // collapses each to a single step. Default to the affordable end so enabling
  // blending alone cannot land on an unusable frame rate.
  node->AddProperty("volumerendering.scattering.reach", mitk::FloatProperty::New(0.0f), renderer, overwrite);
  node->AddProperty("volumerendering.scattering.anisotropy", mitk::FloatProperty::New(0.0f), renderer, overwrite);
  node->AddProperty("volumerendering.normalsFromOpacity", mitk::BoolProperty::New(false), renderer, overwrite);

  node->AddProperty("binary", mitk::BoolProperty::New(false), renderer, overwrite);

  if ((overwrite) || (node->GetProperty("TransferFunction", renderer) == nullptr))
  {
    // add a default transfer function
    mitk::TransferFunction::Pointer tf = mitk::TransferFunction::New();
    mitk::TransferFunctionInitializer::Pointer tfInit = mitk::TransferFunctionInitializer::New(tf);
    tfInit->SetTransferFunctionMode(0);
    node->SetProperty("TransferFunction", mitk::TransferFunctionProperty::New(tf.GetPointer()));
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
  m_SmartVolumeMapper->SetBlendModeToComposite();
  m_SmartVolumeMapper->SetInputData(imageData);
}

void mitk::VolumeMapperVtkSmart3D::createVolume()
{
  m_Volume->SetMapper(m_SmartVolumeMapper);
  m_Volume->SetProperty(m_VolumeProperty);
}

void mitk::VolumeMapperVtkSmart3D::createVolumeProperty()
{
  m_VolumeProperty->ShadeOn();
  m_VolumeProperty->SetInterpolationType(VTK_CUBIC_INTERPOLATION);
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
  m_SmartVolumeMapper->SetRequestedRenderModeToGPU();

  int blendMode;
  if (this->GetDataNode()->GetIntProperty("volumerendering.blendmode", blendMode))
  {
    m_SmartVolumeMapper->SetBlendMode(blendMode);
  }

  // shading parameter
  float value = 0;
  bool shade = true;
  if (this->GetDataNode()->GetFloatProperty("volumerendering.ambient", value, renderer))
    m_VolumeProperty->SetAmbient(value);
  if (this->GetDataNode()->GetFloatProperty("volumerendering.diffuse", value, renderer))
    m_VolumeProperty->SetDiffuse(value);
  if (this->GetDataNode()->GetFloatProperty("volumerendering.specular", value, renderer))
    m_VolumeProperty->SetSpecular(value);
  if (this->GetDataNode()->GetFloatProperty("volumerendering.specular.power", value, renderer))
    m_VolumeProperty->SetSpecularPower(value);
  if(this->GetDataNode()->GetBoolProperty("volumerendering.shade", shade, renderer))
    m_VolumeProperty->SetShade(shade ? 1 : 0);

  // vtkGPUVolumeRayCastMapper drops scattering for interactive frames itself,
  // but decides that on vtkProp::AllocatedRenderTime, which this volume never
  // receives: only vtkMitkRenderProp is a view prop, so vtkRenderer's per-prop
  // time allocation never reaches the volume and it reports the vtkProp default
  // of 10 forever - every frame classed as a still one. The window's update
  // rate does track interaction, and is what vtkSmartVolumeMapper itself keys
  // its sample-distance coarsening off, so gate on that instead.
  //
  // Going through the setter is the point: it calls Modified(), which is what
  // makes VTK recompile the shader without the scattering block. Assigning the
  // value anywhere that skips Modified() leaves the shadow rays compiled in.
  const bool isInteractive = IsInteractiveRender(renderer, m_SmartVolumeMapper->GetInteractiveUpdateRate());

  // VTK ignores the reach unless the blend is above zero, and both unless Shade is on.
  if (this->GetDataNode()->GetFloatProperty("volumerendering.scattering.blend", value, renderer))
    m_SmartVolumeMapper->SetVolumetricScatteringBlending(isInteractive ? 0.0f : value);
  if (this->GetDataNode()->GetFloatProperty("volumerendering.scattering.reach", value, renderer))
    m_SmartVolumeMapper->SetGlobalIlluminationReach(value);

  // Anisotropy feeds only the phase function, which VTK compiles into the
  // shader solely when blending is above zero. Setting it on its own does
  // nothing.
  if (this->GetDataNode()->GetFloatProperty("volumerendering.scattering.anisotropy", value, renderer))
    m_VolumeProperty->SetScatteringAnisotropy(value);

  // Derives the shading gradient from the opacity rather than the raw scalars,
  // so lighting follows the transfer function instead of the data's noise. The
  // blend coefficient reads the same gradient's magnitude, so this also shifts
  // where scattering gives way to Phong shading.
  bool normalsFromOpacity = false;
  if (this->GetDataNode()->GetBoolProperty("volumerendering.normalsFromOpacity", normalsFromOpacity, renderer))
    m_SmartVolumeMapper->SetComputeNormalFromOpacity(normalsFromOpacity);
}

mitk::VolumeMapperVtkSmart3D::VolumeMapperVtkSmart3D()
{
  m_SmartVolumeMapper = vtkSmartPointer<vtkSmartVolumeMapper>::New();
  m_SmartVolumeMapper->SetBlendModeToComposite();
  // Sampling the ray at regular offsets makes the step boundaries line up
  // across neighbouring pixels, which reads as concentric banding. Jittering
  // the offsets trades that for unstructured noise. VTK defaults it off.
  m_SmartVolumeMapper->SetUseJittering(1);
  m_DataToWorld = vtkSmartPointer<vtkTransform>::New();
  m_VolumeProperty = vtkSmartPointer<vtkVolumeProperty>::New();
  m_Volume = vtkSmartPointer<vtkVolume>::New();
}

mitk::VolumeMapperVtkSmart3D::~VolumeMapperVtkSmart3D()
{

}


