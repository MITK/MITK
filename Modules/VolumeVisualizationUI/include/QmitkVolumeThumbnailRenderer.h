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

#include <vtkSmartPointer.h>

#include <QPixmap>
#include <QSize>

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
 * graphics card - so the pipeline is built once and kept; a render afterwards
 * is a fraction of that, and independent of how large the volume is.
 *
 * Rendering happens in an offscreen window of this object's own, not through
 * mitk::RenderingManager, because a preview has one volume, one camera, no
 * interaction and no need to stay in step with anything else.
 *
 * The preview is deliberately not a faithful copy of what the 3D view shows.
 * Lighting is a fixed headlight rig and the blend mode is always composite, so
 * that previews stay comparable with each other and no change to the node can
 * invalidate them. What they do reproduce exactly is the transfer function.
 *
 * The volume ray caster requires a GPU. Where it is unavailable, IsUsable
 * reports false and Render yields null pixmaps rather than failing; callers
 * are expected to fall back to naming the transfer functions instead.
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
   * \return The preview, or a null pixmap if nothing can be drawn.
   */
  QPixmap Render(mitk::TransferFunction *transferFunction);

  /**
   * \brief Whether previews can be drawn at all.
   *
   * False once a bind attempt found no usable volume ray caster. Only ever
   * meaningful after the first SetImage call, since nothing is known before.
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

  vtkSmartPointer<vtkRenderWindow> m_RenderWindow;
  vtkSmartPointer<vtkRenderer> m_Renderer;
  vtkSmartPointer<vtkSmartVolumeMapper> m_Mapper;
  vtkSmartPointer<vtkVolume> m_Volume;
  vtkSmartPointer<vtkVolumeProperty> m_VolumeProperty;
  vtkSmartPointer<vtkWindowToImageFilter> m_Capture;
};

#endif
