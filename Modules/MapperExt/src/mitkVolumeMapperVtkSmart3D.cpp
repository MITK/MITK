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
  auto *localStorage = m_LSH.GetLocalStorage(renderer);

  bool volumeRendering = false;
  this->GetDataNode()->GetBoolProperty("volumerendering", volumeRendering, renderer);

  if (!volumeRendering)
  {
    localStorage->m_Volume->VisibilityOff();
    return;
  }

  auto *imageData = this->GetInputImage();

  // Nothing to draw where the node's data was cleared or is not an image.
  if (nullptr == imageData)
  {
    localStorage->m_Volume->VisibilityOff();
    return;
  }

  localStorage->m_SmartVolumeMapper->SetInputData(imageData);

  this->UpdateTransferFunctions(renderer, localStorage);
  this->UpdateRenderMode(renderer, localStorage);

  localStorage->m_Volume->VisibilityOn();
}

vtkProp* mitk::VolumeMapperVtkSmart3D::GetVtkProp(mitk::BaseRenderer *renderer)
{
  return m_LSH.GetLocalStorage(renderer)->m_Volume;
}

void mitk::VolumeMapperVtkSmart3D::UpdateVtkTransform(mitk::BaseRenderer *renderer)
{
  auto *imageData = this->GetInputImage();

  if (nullptr == imageData)
  {
    Superclass::UpdateVtkTransform(renderer);
    return;
  }

  auto *localStorage = m_LSH.GetLocalStorage(renderer);

  // Read the spacing back from the image the mapper was handed rather than from
  // the geometry, so the two halves of the placement cannot disagree if the
  // geometry is rescaled after the image data was built.
  double spacing[3];
  imageData->GetSpacing(spacing);

  // A node carrying no geometry for this timestep yields no transform. The base
  // class hands that to SetUserTransform, which reads it as "no transform", so
  // deferring to it here keeps the tolerance this override would otherwise drop.
  auto *indexToWorld = this->GetDataNode()->GetVtkTransform(this->GetTimestep());

  if (nullptr == indexToWorld)
  {
    Superclass::UpdateVtkTransform(renderer);
    return;
  }

  // IndexToWorld carries the spacing the image itself now supplies. Applying
  // both would size the volume by it twice.
  localStorage->m_DataToWorld->SetMatrix(indexToWorld->GetMatrix());
  localStorage->m_DataToWorld->Scale(1.0 / spacing[0], 1.0 / spacing[1], 1.0 / spacing[2]);

  localStorage->m_Volume->SetUserTransform(localStorage->m_DataToWorld);
}

void mitk::VolumeMapperVtkSmart3D::SetDefaultProperties(mitk::DataNode *node, mitk::BaseRenderer *renderer, bool overwrite)
{
  // GPU_INFO << "SetDefaultProperties";

  const auto *image = dynamic_cast<const mitk::Image *>(node->GetData());

  if (image == nullptr || !image->IsInitialized())
    return;

  node->AddProperty("volumerendering", mitk::BoolProperty::New(false), renderer, overwrite);
  node->AddProperty(
    "volumerendering.blendmode", mitk::IntProperty::New(vtkVolumeMapper::COMPOSITE_BLEND), renderer, overwrite);

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

  // UpdateVtkTransform calls this before anything has established that the node
  // still holds an image, and it runs whether or not rendering is switched on.
  if (nullptr == input)
    return nullptr;

  return input->GetVtkImageData(this->GetTimestep());
}

void mitk::VolumeMapperVtkSmart3D::UpdateTransferFunctions(mitk::BaseRenderer *renderer, LocalStorage *localStorage)
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
  localStorage->m_VolumeProperty->SetColor(colorTransferFunction);
  localStorage->m_VolumeProperty->SetScalarOpacity(opacityTransferFunction);
  localStorage->m_VolumeProperty->SetGradientOpacity(gradientTransferFunction);
}


void mitk::VolumeMapperVtkSmart3D::UpdateRenderMode(mitk::BaseRenderer *renderer, LocalStorage *localStorage)
{
  int blendMode = vtkVolumeMapper::COMPOSITE_BLEND;
  const bool hasBlendMode =
    this->GetDataNode()->GetIntProperty("volumerendering.blendmode", blendMode, renderer);

  // Range-checked because the property is a plain int anything can write: the
  // Properties view offers it as one, and a scene file carries whatever it was
  // saved with. VTK's setter does not validate, and its ray caster refuses the
  // entire render for an out-of-enum mode, so such a value makes the volume
  // vanish rather than degrade. Isosurface and slice are excluded for a
  // different reason: the ray caster accepts both, but isosurface draws nothing
  // without iso-values and slice nothing without a plane, and this mapper
  // supplies neither.
  //
  // mitk::VolumeBlendMode enumerates this same set from the other side, for the
  // views that write the property. The two are kept in step by hand, because the
  // module holding it sits above this one and this file cannot name it. Adding
  // either excluded mode there would mean supplying its input here anyway.
  const bool renderable = blendMode >= vtkVolumeMapper::COMPOSITE_BLEND &&
                          blendMode <= vtkVolumeMapper::ADDITIVE_BLEND;

  if (hasBlendMode && !renderable && blendMode != localStorage->m_ReportedBlendMode)
  {
    // Once per value rather than per frame: this runs on every render pass.
    MITK_WARN << "Volume rendering blend mode " << blendMode
              << " cannot be rendered; falling back to composite.";
    localStorage->m_ReportedBlendMode = blendMode;
  }

  if (hasBlendMode && renderable)
  {
    localStorage->m_SmartVolumeMapper->SetBlendMode(blendMode);
  }
  else
  {
    // Restored rather than left alone: the ray caster keeps the last mode it was
    // handed, so a node that loses the property, or names an unusable one, would
    // stay on it.
    localStorage->m_SmartVolumeMapper->SetBlendModeToComposite();
    blendMode = vtkVolumeMapper::COMPOSITE_BLEND;
  }

  // shading parameter
  float value = 0;
  bool shade = true;
  if (this->GetDataNode()->GetFloatProperty("volumerendering.ambient", value, renderer))
    localStorage->m_VolumeProperty->SetAmbient(value);
  if (this->GetDataNode()->GetFloatProperty("volumerendering.diffuse", value, renderer))
    localStorage->m_VolumeProperty->SetDiffuse(value);
  if (this->GetDataNode()->GetFloatProperty("volumerendering.specular", value, renderer))
    localStorage->m_VolumeProperty->SetSpecular(value);
  if (this->GetDataNode()->GetFloatProperty("volumerendering.specular.power", value, renderer))
    localStorage->m_VolumeProperty->SetSpecularPower(value);
  if(this->GetDataNode()->GetBoolProperty("volumerendering.shade", shade, renderer))
    localStorage->m_VolumeProperty->SetShade(shade ? 1 : 0);

  // Scattering traces shadow rays, far too slow to rotate with, so it has to
  // switch off while the user drags. VTK does try that itself, but bases it on
  // vtkProp::AllocatedRenderTime, and vtkRenderer only budgets props that were
  // added to it - here that is vtkMitkRenderProp alone, never the volume. So
  // the volume keeps the vtkProp default of 10 and never counts as interactive.
  // The render window's update rate does follow interaction, and
  // vtkSmartVolumeMapper already keys its own interactive coarsening off it.
  //
  // The blend has to go through the setter rather than be assigned: only the
  // setter calls Modified(), and VTK rebuilds the shader by comparing the
  // mapper's modification time against the last build. Scattering is compiled
  // into the shader, so without that bump the shadow rays keep running no
  // matter what the value says.
  const bool isInteractive =
    IsInteractiveRender(renderer, localStorage->m_SmartVolumeMapper->GetInteractiveUpdateRate());

  // Suppressed outside the composite path, not merely ineffective there: VTK
  // splices the scattering code into whatever shader it is building, but that
  // code reads variables - the view direction and the shading gradient - that
  // only the compositing loop declares. Handing a non-zero blend to any
  // projection mode therefore emits GLSL that fails to compile, and the volume
  // vanishes rather than rendering unlit. The node keeps its lighting model
  // through the excursion; only what reaches the ray caster is held back.
  const bool composites = blendMode == vtkVolumeMapper::COMPOSITE_BLEND;

  // VTK ignores the reach unless the blend is above zero, and both unless Shade is on.
  if (this->GetDataNode()->GetFloatProperty("volumerendering.scattering.blend", value, renderer))
    localStorage->m_SmartVolumeMapper->SetVolumetricScatteringBlending(
      isInteractive || !composites ? 0.0f : value);
  if (this->GetDataNode()->GetFloatProperty("volumerendering.scattering.reach", value, renderer))
    localStorage->m_SmartVolumeMapper->SetGlobalIlluminationReach(value);

  // Anisotropy feeds only the phase function, which VTK compiles into the
  // shader solely when blending is above zero. Setting it on its own does
  // nothing.
  if (this->GetDataNode()->GetFloatProperty("volumerendering.scattering.anisotropy", value, renderer))
    localStorage->m_VolumeProperty->SetScatteringAnisotropy(value);

  // Derives the shading gradient from the opacity rather than the raw scalars,
  // so lighting follows the transfer function instead of the data's noise. The
  // blend coefficient reads the same gradient's magnitude, so this also shifts
  // where scattering gives way to Phong shading.
  bool normalsFromOpacity = false;
  if (this->GetDataNode()->GetBoolProperty("volumerendering.normalsFromOpacity", normalsFromOpacity, renderer))
    localStorage->m_SmartVolumeMapper->SetComputeNormalFromOpacity(normalsFromOpacity);
}

mitk::VolumeMapperVtkSmart3D::LocalStorage::LocalStorage()
{
  m_SmartVolumeMapper = vtkSmartPointer<vtkSmartVolumeMapper>::New();
  // Requested explicitly rather than left to VTK's own selection: the jittering
  // below, and the scattering and normals-from-opacity in UpdateRenderMode, are
  // honoured only by the GPU ray caster. Letting VTK settle on the CPU mapper
  // would drop them silently rather than degrade.
  m_SmartVolumeMapper->SetRequestedRenderModeToGPU();
  m_SmartVolumeMapper->SetBlendModeToComposite();
  // Sampling the ray at regular offsets makes the step boundaries line up
  // across neighbouring pixels, which reads as concentric banding. Jittering
  // the offsets trades that for unstructured noise. VTK defaults it off.
  m_SmartVolumeMapper->SetUseJittering(1);

  m_VolumeProperty = vtkSmartPointer<vtkVolumeProperty>::New();
  m_VolumeProperty->ShadeOn();
  // vtkVolumeProperty supports nearest and linear only, and defaults to
  // nearest. Higher values are silently clamped, so cubic cannot be had here.
  m_VolumeProperty->SetInterpolationTypeToLinear();

  m_DataToWorld = vtkSmartPointer<vtkTransform>::New();

  m_Volume = vtkSmartPointer<vtkVolume>::New();
  m_Volume->SetMapper(m_SmartVolumeMapper);
  m_Volume->SetProperty(m_VolumeProperty);

  // GetVtkProp hands this prop out before GenerateDataForRenderer has ever run,
  // and until it does the ray caster has no input to render.
  m_Volume->VisibilityOff();
}

mitk::VolumeMapperVtkSmart3D::LocalStorage::~LocalStorage()
{
}

mitk::VolumeMapperVtkSmart3D::VolumeMapperVtkSmart3D()
{
}

mitk::VolumeMapperVtkSmart3D::~VolumeMapperVtkSmart3D()
{
}


