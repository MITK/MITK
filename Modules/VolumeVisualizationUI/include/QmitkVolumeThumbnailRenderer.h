/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkVolumeThumbnailRenderer_h
#define QmitkVolumeThumbnailRenderer_h

#include <MitkVolumeVisualizationUIExports.h>

#include <mitkVolumeBlendMode.h>
#include <mitkVolumeRenderingScalarRange.h>

#include <itkSmartPointer.h>

#include <vtkSmartPointer.h>

#include <QPixmap>
#include <QSize>

class vtkImageData;
class vtkRenderer;
class vtkRenderWindow;
class vtkSmartVolumeMapper;
class vtkVolume;
class vtkVolumeProperty;
class vtkWindowToImageFilter;

namespace mitk
{
  class Image;
  class TransferFunction;
}

/**
 * \brief Draws small previews of one volume under changing transfer functions.
 *
 * Bind an image once with SetImage, then call Render for each transfer
 * function to preview. Binding is what costs - it uploads the volume to the
 * graphics card - so the pipeline is built once and kept.
 *
 * A render afterwards is a fraction of that, except where the transfer
 * function is authored over a different intensity range than the one before
 * it. The ray caster fills its lookup tables over the range of the image it is
 * handed, so a preview is drawn through a view of the volume reporting the
 * range of the curve being previewed (see mitk::ViewWithScalarRange), without
 * which a curve authored for one modality is resolved by a handful of table
 * entries on an image from another. Changing that view costs the upload again,
 * so previews of curves sharing a range are cheapest drawn together.
 *
 * Rendering happens in an offscreen window of this object's own, not through
 * mitk::RenderingManager, because a preview has one volume, one camera, no
 * interaction and no need to stay in step with anything else.
 *
 * The preview is deliberately not a faithful copy of what the 3D view shows.
 * Lighting is a fixed headlight rig, so that previews stay comparable with each
 * other and no change to the node can invalidate them. What they do reproduce
 * exactly is the transfer function and the blend mode it was authored for -
 * which cannot be fixed the way lighting is, since a curve drawn as a window
 * renders as a white shell under composite and says nothing about itself.
 *
 * The volume ray caster requires a GPU, and refuses volumes it cannot take,
 * RGB ones among them. Where a bind is refused, SetImage returns false and
 * Render yields null pixmaps rather than failing; callers are expected to fall
 * back to naming the transfer functions instead. A refusal describes the image
 * that drew it, so the next image is still bound on its own merits.
 */
class MITKVOLUMEVISUALIZATIONUI_EXPORT QmitkVolumeThumbnailRenderer
{
public:
  /**
   * \brief Create a renderer drawing previews of the given pixel size.
   *
   * No graphics resources are claimed until the first SetImage call.
   *
   * \param[in] size The pixel size of every pixmap Render returns. Callers
   *            showing previews smaller than this let Qt scale them down,
   *            which costs nothing next to drawing them again.
   */
  explicit QmitkVolumeThumbnailRenderer(const QSize &size);

  ~QmitkVolumeThumbnailRenderer();

  QmitkVolumeThumbnailRenderer(const QmitkVolumeThumbnailRenderer &) = delete;
  QmitkVolumeThumbnailRenderer &operator =(const QmitkVolumeThumbnailRenderer &) = delete;

  /**
   * \brief Bind the volume that every later Render call draws.
   *
   * Uploads the volume and proves the graphics card can draw it, so this is
   * the expensive call. Re-binding the same image is free.
   *
   * \param[in] image The image to draw; nullptr releases the bound volume.
   * \return True if the volume can be drawn.
   */
  bool SetImage(const mitk::Image *image);

  /**
   * \brief Draw the bound volume with one transfer function.
   *
   * \param[in] transferFunction The colour and opacity to draw with.
   * \param[in] blendMode The mode that transfer function was authored for.
   *            Taken alongside the function rather than set once, because the
   *            two only mean anything together.
   * \return The preview, or a null pixmap if nothing can be drawn.
   */
  QPixmap Render(mitk::TransferFunction *transferFunction, mitk::VolumeBlendMode blendMode);

  /**
   * \brief Whether the volume bound last can be drawn.
   *
   * False once a bind was refused, which the ray caster does for a volume it
   * cannot take as readily as on a machine that can take none. It describes
   * that one image, so it must not be used to decide whether to bind the next:
   * SetImage answers that question per image and reports the answer itself.
   * Meaningful only after the first SetImage call, since nothing is known
   * before one.
   */
  bool IsUsable() const;

private:
  /** \brief Build the offscreen pipeline, once. */
  void CreatePipeline();

  QSize m_Size;
  bool m_Usable = true;

  /** \brief Whether the missing ray caster has already been reported.
   *
   * Binding happens on every image change, so without this the same complaint
   * would be logged for the lifetime of the session.
   */
  bool m_ReportedUnusable = false;

  /** \brief The image bound last.
   *
   * Held for its voxels: the representation below does not own them, and
   * previews are drawn across turns of the event loop, so the image has to
   * outlive the last of them rather than the call that bound it.
   */
  itk::SmartPointer<const mitk::Image> m_Image;

  /** \brief Its VTK representation, which every preview is a view of. */
  vtkSmartPointer<vtkImageData> m_ImageData;

  /** \brief The view of it the last preview was drawn through. */
  mitk::ScalarRangeViewCache m_ViewCache;

  vtkSmartPointer<vtkRenderWindow> m_RenderWindow;
  vtkSmartPointer<vtkRenderer> m_Renderer;
  vtkSmartPointer<vtkSmartVolumeMapper> m_Mapper;
  vtkSmartPointer<vtkVolume> m_Volume;
  vtkSmartPointer<vtkVolumeProperty> m_VolumeProperty;
  vtkSmartPointer<vtkWindowToImageFilter> m_Capture;
};

#endif
