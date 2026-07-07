/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef vtkMitkRenderProp_h
#define vtkMitkRenderProp_h

#include <mitkVtkPropRenderer.h>
#include <vtkProp.h>

/**
 * \brief Custom vtkProp that bridges the MITK rendering process into the VTK pipeline.
 *
 * The MITK rendering process is completely integrated into the VTK rendering pipeline.
 * vtkMitkRenderProp is a custom vtkProp-derived class that implements the rendering
 * interface between MITK and VTK. It redirects VTK's various Render..Geometry() calls
 * to mitk::VtkPropRenderer, which is responsible for rendering of mitk::DataNodes.
 *
 * \sa mitk::VtkPropRenderer
 */
class MITKCORE_EXPORT vtkMitkRenderProp : public vtkProp
{
public:
  /** \brief Create a new instance of vtkMitkRenderProp. */
  static vtkMitkRenderProp *New();
  vtkTypeMacro(vtkMitkRenderProp, vtkProp);

  /**
   * \brief Set the MITK VtkPropRenderer that this prop delegates to.
   *
   * \param[in] propRenderer  The VtkPropRenderer to use for rendering.
   */
  void SetPropRenderer(mitk::VtkPropRenderer::Pointer propRenderer);

  /**
   * \brief Store a vtkInformation object and forward it to all mitk::VtkMapper props.
   *
   * \param[in] keys  The vtkInformation object containing property keys.
   */
  void SetPropertyKeys(vtkInformation *keys) override;

  /**
   * \brief Render opaque geometry by delegating to VtkPropRenderer.
   *
   * \param[in] viewport  The viewport to render into.
   * \return The render result from VtkPropRenderer.
   */
  int RenderOpaqueGeometry(vtkViewport *viewport) override;

  /**
   * \brief Render overlay geometry by delegating to VtkPropRenderer.
   *
   * \param[in] viewport  The viewport to render into.
   * \return The render result from VtkPropRenderer.
   */
  int RenderOverlay(vtkViewport *viewport) override;

  /**
   * \brief Get the bounds from the VtkPropRenderer.
   *
   * \return Pointer to a six-element array (xmin, xmax, ymin, ymax, zmin, zmax).
   */
  double *GetBounds() override;

  /**
   * \brief Release graphics resources by delegating to VtkPropRenderer.
   *
   * \param[in] window  The vtkWindow whose resources should be released.
   */
  void ReleaseGraphicsResources(vtkWindow *window) override;

  /**
   * \brief Initialize path traversal for vtkPointPicker/vtkPicker support.
   *
   * Queries all objects in MITK and provides every VTK-based mapper to the picker.
   */
  void InitPathTraversal() override;

  /**
   * \brief Get the next assembly path for vtkPointPicker/vtkPicker support.
   *
   * \return The next vtkAssemblyPath, or nullptr if no more paths are available.
   */
  vtkAssemblyPath *GetNextPath() override;

  /**
   * \brief Get the number of assembly paths.
   *
   * \return The number of paths from VtkPropRenderer.
   */
  int GetNumberOfPaths() override;

  /**
   * \brief Check if any mapper has translucent polygonal geometry.
   *
   * \return 1 if any mapper has translucent geometry, 0 otherwise.
   */
  int HasTranslucentPolygonalGeometry() override;

  /**
   * \brief Render translucent polygonal geometry by delegating to VtkPropRenderer.
   *
   * \return The render result from VtkPropRenderer.
   */
  int RenderTranslucentPolygonalGeometry(vtkViewport *) override;

  /**
   * \brief Render volumetric geometry by delegating to VtkPropRenderer.
   *
   * \return The render result from VtkPropRenderer.
   */
  int RenderVolumetricGeometry(vtkViewport *) override;

protected:
  /** \brief Constructor. */
  vtkMitkRenderProp();

  /** \brief Destructor. */
  ~vtkMitkRenderProp() override;

  mitk::VtkPropRenderer::Pointer m_VtkPropRenderer;
};

#endif /* vtkMitkRenderProp_h */
