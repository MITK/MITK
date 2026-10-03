/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkVolumeThumbnailRenderer.h"

#include <mitkExceptionMacro.h>
#include <mitkImage.h>
#include <mitkImageVtkReadView.h>
#include <mitkLog.h>
#include <mitkTransferFunction.h>
#include <mitkVolumeRenderingLightingModel.h>

#include <vtkCamera.h>
#include <vtkImageData.h>
#include <vtkImageShrink3D.h>
#include <vtkLight.h>
#include <vtkMatrix4x4.h>
#include <vtkObject.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkSmartVolumeMapper.h>
#include <vtkTransform.h>
#include <vtkVolume.h>
#include <vtkVolumeProperty.h>
#include <vtkWindowToImageFilter.h>

#include <QImage>

#include <algorithm>

namespace
{
  /** \brief The lighting every preview is drawn with.
   *
   * Fixed rather than read from the node: previews are only useful next to
   * each other, and a preview that changed with the node's lighting model
   * would have to be redrawn whenever that model changed.
   */
  constexpr const char *THUMBNAIL_LIGHTING_MODEL = "headlight";

  /** \brief How much wider than the volume's box the view is, where it fits tightest.
   *
   * A scan that starts and ends mid-body, as most CTs do, is bounded by flat
   * cross sections. Fitted exactly, they coincide with the preview's edges and
   * read as the preview cropping the body. A little background between them
   * and the edge is what lets them read as the ends of the data instead.
   */
  constexpr double PREVIEW_MARGIN = 0.05;

  /** \brief The most voxels a preview volume has along any axis.
   *
   * Above the widest preview drawn, so that no axis is resolved more coarsely
   * than the pixels it is drawn onto.
   */
  constexpr int MAX_PREVIEW_VOXELS = 256;

  /** \brief Place the volume without counting its spacing twice.
   *
   * The voxels carry the geometry's spacing, and IndexToWorld carries it
   * again, so the transform has to divide it back out.
   * mitk::VolumeMapperVtkSmart3D::UpdateVtkTransform does the same for the
   * mapper that draws into the real render windows.
   */
  vtkSmartPointer<vtkTransform> CreateDataToWorldTransform(vtkMatrix4x4 *indexToWorld, vtkImageData *imageData)
  {
    double spacing[3];
    imageData->GetSpacing(spacing);

    auto dataToWorld = vtkSmartPointer<vtkTransform>::New();
    dataToWorld->SetMatrix(indexToWorld);
    dataToWorld->Scale(1.0 / spacing[0], 1.0 / spacing[1], 1.0 / spacing[2]);

    return dataToWorld;
  }

  /** \brief Copy a captured frame out of VTK's buffer as a pixmap.
   *
   * \return The frame, or a null pixmap for anything but tightly packed RGB.
   */
  QPixmap ConvertToPixmap(vtkImageData *frame)
  {
    if (frame == nullptr ||
        frame->GetScalarType() != VTK_UNSIGNED_CHAR ||
        frame->GetNumberOfScalarComponents() != 3)
    {
      return QPixmap();
    }

    int dimensions[3];
    frame->GetDimensions(dimensions);

    if (dimensions[0] <= 0 || dimensions[1] <= 0)
      return QPixmap();

    // The stride has to be given: QImage would otherwise assume rows padded to
    // four bytes, which a tightly packed RGB row only happens to be when the
    // width is a multiple of four.
    const QImage frameImage(static_cast<const uchar *>(frame->GetScalarPointer()),
                            dimensions[0], dimensions[1], dimensions[0] * 3, QImage::Format_RGB888);

    // VTK numbers rows from the bottom, Qt from the top. Flipping also detaches
    // from VTK's buffer, which the next capture overwrites.
    return QPixmap::fromImage(frameImage.flipped(Qt::Vertical));
  }
}

QmitkVolumeThumbnailRenderer::Volume QmitkVolumeThumbnailRenderer::CreateVolume(const mitk::Image *image)
{
  if (image == nullptr || !image->IsInitialized())
    mitkThrow() << "Cannot create a preview volume without an initialized image.";

  // A view of its own rather than Image::GetVtkImageData(), which the mappers
  // share and which feeding into the filter below would modify.
  const mitk::ImageVtkReadView view(image, 0);
  auto *imageData = view.GetVtkImageData();

  int dimensions[3];
  imageData->GetDimensions(dimensions);

  int factors[3];

  for (int i = 0; i < 3; ++i)
    factors[i] = std::max(1, (dimensions[i] + MAX_PREVIEW_VOXELS - 1) / MAX_PREVIEW_VOXELS);

  auto shrink = vtkSmartPointer<vtkImageShrink3D>::New();
  shrink->SetInputData(imageData);
  shrink->SetShrinkFactors(factors);
  shrink->AveragingOn();
  shrink->Update();

  // Detached from the filter, so that binding it does not drag the filter's
  // pipeline, and the view it reads, along.
  Volume volume;
  volume.ImageData = vtkSmartPointer<vtkImageData>::New();
  volume.ImageData->ShallowCopy(shrink->GetOutput());
  volume.ImageData->SetOrigin(0.0, 0.0, 0.0);

  // A preview voxel averages a block of the image's voxels, so it stands at
  // that block's center: index j of the preview is index f * j + (f - 1) / 2
  // of the image along an axis shrunk by f.
  auto *imageIndexToWorld = image->GetGeometry()->GetVtkTransform()->GetMatrix();

  volume.IndexToWorld = vtkSmartPointer<vtkMatrix4x4>::New();
  volume.IndexToWorld->DeepCopy(imageIndexToWorld);

  for (int row = 0; row < 3; ++row)
  {
    double offset = imageIndexToWorld->GetElement(row, 3);

    for (int column = 0; column < 3; ++column)
    {
      const double element = imageIndexToWorld->GetElement(row, column);

      volume.IndexToWorld->SetElement(row, column, element * factors[column]);
      offset += element * 0.5 * (factors[column] - 1);
    }

    volume.IndexToWorld->SetElement(row, 3, offset);
  }

  return volume;
}

QmitkVolumeThumbnailRenderer::QmitkVolumeThumbnailRenderer(const QSize &size)
  : m_Size(size)
{
}

QmitkVolumeThumbnailRenderer::~QmitkVolumeThumbnailRenderer() = default;

bool QmitkVolumeThumbnailRenderer::IsUsable() const
{
  return m_Usable;
}

void QmitkVolumeThumbnailRenderer::CreatePipeline()
{
  m_RenderWindow = vtkSmartPointer<vtkRenderWindow>::New();

  // The pair SetOffScreenRendering() sets. vtkWindow marks the combined getter
  // deprecated in favor of asking about the two separately.
  m_RenderWindow->SetShowWindow(false);
  m_RenderWindow->SetUseOffScreenBuffers(true);
  m_RenderWindow->SetSize(m_Size.width(), m_Size.height());

  // A window left at VTK's default update rate is taken for a still frame and
  // drawn at the finest sample distance the ray caster offers. Asking for an
  // interactive rate instead is what makes a preview cost a few milliseconds.
  m_RenderWindow->SetDesiredUpdateRate(10.0);

  m_Renderer = vtkSmartPointer<vtkRenderer>::New();
  m_RenderWindow->AddRenderer(m_Renderer);

  // One headlight at intensity 1.0 and nothing else: that exact rig is what
  // selects the ray caster's default lighting path, where ambient is tinted by
  // the sample's own color rather than laid over the picture as flat gray. It
  // is also the rig mitk::VtkPropRenderer installs for the headlight model, so
  // a preview is lit the way the 3D window will light the node.
  //
  // Ambient color 1.0 is what VtkPropRenderer's normalization works out to for
  // that rig. This lighting path never reads it, but the volumetric scattering
  // path does, so setting it keeps the two rigs identical and the preview right
  // if scattering is ever switched on here.
  auto light = vtkSmartPointer<vtkLight>::New();
  light->SetLightTypeToHeadlight();
  light->SetIntensity(1.0);
  light->SetAmbientColor(1.0, 1.0, 1.0);
  m_Renderer->AddLight(light);

  m_Mapper = vtkSmartPointer<vtkSmartVolumeMapper>::New();
  m_Mapper->SetRequestedRenderModeToGPU();

  m_VolumeProperty = vtkSmartPointer<vtkVolumeProperty>::New();

  // VTK defaults to nearest neighbor, which at preview size reads as blocky.
  m_VolumeProperty->SetInterpolationTypeToLinear();
  m_VolumeProperty->ShadeOn();

  if (const auto *model = mitk::VolumeRenderingLightingModel::FromId(THUMBNAIL_LIGHTING_MODEL);
      model != nullptr)
  {
    m_VolumeProperty->SetAmbient(model->ambient);
    m_VolumeProperty->SetDiffuse(model->diffuse);
    m_VolumeProperty->SetSpecular(model->specular);
    m_VolumeProperty->SetSpecularPower(model->specularPower);
  }

  m_Volume = vtkSmartPointer<vtkVolume>::New();
  m_Volume->SetMapper(m_Mapper);
  m_Volume->SetProperty(m_VolumeProperty);
  m_Renderer->AddVolume(m_Volume);

  m_Capture = vtkSmartPointer<vtkWindowToImageFilter>::New();
  m_Capture->SetInput(m_RenderWindow);

  // The frame is in the back buffer, and Render has already been called by the
  // time the frame is asked for, so the filter must not draw it a second time.
  m_Capture->ReadFrontBufferOff();
  m_Capture->ShouldRerenderOff();
}

bool QmitkVolumeThumbnailRenderer::SetVolume(const Volume &volume)
{
  if (volume.ImageData == nullptr || volume.IndexToWorld == nullptr)
  {
    m_ImageData = nullptr;

    // Its view shares the voxels just released, so it has to go with them.
    m_ViewCache.Reset();

    if (m_Mapper != nullptr)
      m_Mapper->RemoveAllInputs();

    return false;
  }

  // Deferred to here so that a session in which nobody asks for a preview never
  // claims a graphics context.
  if (m_RenderWindow == nullptr)
    this->CreatePipeline();

  // The cached view belongs to the voxels this bind is about to release.
  if (m_ImageData != volume.ImageData)
    m_ViewCache.Reset();

  // Kept for Render to build its views of, which is what the previews are
  // drawn through. This bind carries no transfer function yet, so the volume
  // itself is what proves the ray caster can draw it.
  m_ImageData = volume.ImageData;

  m_Mapper->SetInputData(m_ImageData);
  m_Volume->SetUserTransform(CreateDataToWorldTransform(volume.IndexToWorld, m_ImageData));

  // Parallel rather than perspective, since only then does the volume's box
  // project to exactly its width and height, which is what the view is sized
  // to below. At preview size the perspective of the 3D window is too slight
  // to be missed.
  auto *camera = m_Renderer->GetActiveCamera();
  camera->ParallelProjectionOn();
  camera->SetPosition(0.0, -1.0, 0.0);
  camera->SetFocalPoint(0.0, 0.0, 0.0);
  camera->SetViewUp(0.0, 0.0, 1.0);
  m_Renderer->ResetCamera();

  // ResetCamera sizes the view to the sphere around the volume's box, whose
  // radius is half the box's full diagonal, depth included, and that leaves the
  // box filling well under half of a preview. Sized to the box's own width and
  // height instead, it stops just short of the preview's edges in whichever
  // direction runs out first, and still nothing of it can be cropped. The
  // camera looks along +Y, so X runs across the picture and Z up it. The
  // clipping range ResetCamera computed stays valid, since the camera does not
  // move.
  double bounds[6];
  m_Volume->GetBounds(bounds);

  const double halfWidth = 0.5 * (bounds[1] - bounds[0]);
  const double halfHeight = 0.5 * (bounds[5] - bounds[4]);
  const double aspect = static_cast<double>(m_Size.width()) / m_Size.height();

  // The parallel scale is half the height of the view.
  if (const double scale = std::max(halfHeight, halfWidth / aspect); scale > 0.0)
    camera->SetParallelScale(scale * (1.0 + PREVIEW_MARGIN));

  // Uploads the volume, and is the only expensive call here. Whether the ray
  // caster can draw at all depends on the hardware and on this volume's own
  // scalar components, so it can only be answered by rendering and asking
  // afterwards which mapper ran. VTK reports its own refusal loudly; one
  // warning of our own is more use than its stack.
  const int warningDisplay = vtkObject::GetGlobalWarningDisplay();
  vtkObject::GlobalWarningDisplayOff();
  m_RenderWindow->Render();
  vtkObject::SetGlobalWarningDisplay(warningDisplay);

  m_Usable = m_Mapper->GetLastUsedRenderMode() == vtkSmartVolumeMapper::GPURenderMode;

  if (!m_Usable && !m_ReportedUnusable)
  {
    MITK_WARN << "No GPU volume ray caster available; transfer function previews are disabled.";
    m_ReportedUnusable = true;
  }

  return m_Usable;
}

QPixmap QmitkVolumeThumbnailRenderer::Render(
  mitk::TransferFunction *transferFunction, mitk::VolumeBlendMode blendMode)
{
  if (!m_Usable || m_RenderWindow == nullptr || m_ImageData == nullptr || transferFunction == nullptr)
    return QPixmap();

  m_Mapper->SetBlendMode(mitk::ToVtkBlendMode(blendMode));

  m_VolumeProperty->SetColor(transferFunction->GetColorTransferFunction());
  m_VolumeProperty->SetScalarOpacity(transferFunction->GetScalarOpacityFunction());

  // Every preset is offered for every image, so one authored for a narrow
  // intensity range is regularly drawn over data far wider. Through a view
  // reporting the curve's own range, the ray caster resolves it as it was
  // authored whatever the image holds.
  m_Mapper->SetInputData(m_ViewCache.GetView(m_ImageData, m_VolumeProperty));

  m_RenderWindow->Render();

  // The filter holds on to the frame it last read, and the render window is not
  // one of the inputs it notices changing, so it has to be told each time.
  m_Capture->Modified();
  m_Capture->Update();

  return ConvertToPixmap(m_Capture->GetOutput());
}
