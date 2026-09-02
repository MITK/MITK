/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkVolumeThumbnailRenderer.h"

#include <mitkImage.h>
#include <mitkLog.h>
#include <mitkTransferFunction.h>
#include <mitkVolumeRenderingLightingModel.h>

#include <vtkCamera.h>
#include <vtkImageData.h>
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

namespace
{
  /** \brief The lighting every preview is drawn with.
   *
   * Fixed rather than read from the node: previews are only useful next to
   * each other, and a preview that changed with the node's lighting model
   * would have to be redrawn whenever that model changed.
   */
  constexpr const char *THUMBNAIL_LIGHTING_MODEL = "headlight";

  /** \brief Place the volume without counting its spacing twice.
   *
   * The vtkImageData MITK hands out carries the geometry's spacing, and
   * IndexToWorld carries it again, so the transform has to divide it back out.
   * mitk::VolumeMapperVtkSmart3D::UpdateVtkTransform does the same for the
   * mapper that draws into the real render windows.
   */
  vtkSmartPointer<vtkTransform> CreateDataToWorldTransform(const mitk::Image *image, vtkImageData *imageData)
  {
    double spacing[3];
    imageData->GetSpacing(spacing);

    auto dataToWorld = vtkSmartPointer<vtkTransform>::New();
    dataToWorld->SetMatrix(image->GetGeometry()->GetVtkTransform()->GetMatrix());
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
  // deprecated in favour of asking about the two separately.
  m_RenderWindow->SetShowWindow(false);
  m_RenderWindow->SetUseOffScreenBuffers(true);
  m_RenderWindow->SetSize(m_Size.width(), m_Size.height());

  // A window left at VTK's default update rate is taken for a still frame and
  // drawn at the finest sample distance the ray caster offers. Asking for an
  // interactive rate instead is what makes a preview cost a few milliseconds.
  m_RenderWindow->SetDesiredUpdateRate(10.0);

  m_Renderer = vtkSmartPointer<vtkRenderer>::New();
  m_RenderWindow->AddRenderer(m_Renderer);

  // vtkLight leaves its ambient colour black, and the ray caster multiplies the
  // volume's ambient coefficient by it, so without this the coefficient set
  // below would have no effect at all. One light at full intensity makes the
  // normalisation mitk::VtkPropRenderer applies to its own rigs come out at 1.
  auto light = vtkSmartPointer<vtkLight>::New();
  light->SetLightTypeToHeadlight();
  light->SetIntensity(1.0);
  light->SetAmbientColor(1.0, 1.0, 1.0);
  m_Renderer->AddLight(light);

  m_Mapper = vtkSmartPointer<vtkSmartVolumeMapper>::New();
  m_Mapper->SetRequestedRenderModeToGPU();

  // Previews show what a transfer function does, not how the node is projected,
  // so a node set to maximum intensity still gets composited previews.
  m_Mapper->SetBlendModeToComposite();

  m_VolumeProperty = vtkSmartPointer<vtkVolumeProperty>::New();

  // VTK defaults to nearest neighbour, which at preview size reads as blocky.
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

bool QmitkVolumeThumbnailRenderer::SetImage(const mitk::Image *image)
{
  if (image == nullptr || !image->IsInitialized())
  {
    if (m_Mapper != nullptr)
      m_Mapper->RemoveAllInputs();

    return false;
  }

  // The const overload registers a read accessor rather than a write one, which
  // is what a consumer that only draws should leave behind. VTK's setter takes
  // a mutable pointer, hence the cast.
  auto *imageData = const_cast<vtkImageData *>(image->GetVtkImageData());

  if (imageData == nullptr)
    return false;

  // Deferred to here so that a session in which nobody asks for a preview never
  // claims a graphics context.
  if (m_RenderWindow == nullptr)
    this->CreatePipeline();

  m_Mapper->SetInputData(imageData);
  m_Volume->SetUserTransform(CreateDataToWorldTransform(image, imageData));

  auto *camera = m_Renderer->GetActiveCamera();
  camera->SetPosition(0.0, -1.0, 0.0);
  camera->SetFocalPoint(0.0, 0.0, 0.0);
  camera->SetViewUp(0.0, 0.0, 1.0);
  m_Renderer->ResetCamera();

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

QPixmap QmitkVolumeThumbnailRenderer::Render(mitk::TransferFunction *transferFunction)
{
  if (!m_Usable || m_RenderWindow == nullptr || transferFunction == nullptr)
    return QPixmap();

  m_VolumeProperty->SetColor(transferFunction->GetColorTransferFunction());
  m_VolumeProperty->SetScalarOpacity(transferFunction->GetScalarOpacityFunction());

  m_RenderWindow->Render();

  // The filter holds on to the frame it last read, and the render window is not
  // one of the inputs it notices changing, so it has to be told each time.
  m_Capture->Modified();
  m_Capture->Update();

  return ConvertToPixmap(m_Capture->GetOutput());
}
