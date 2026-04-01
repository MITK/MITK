/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef vtkGLMapperProp_h
#define vtkGLMapperProp_h

#include <MitkLegacyGLExports.h>

#include <mitkGLMapper.h>
#include <vtkProp.h>
#include <vtkSmartPointer.h>

/** \brief VTK prop that wraps a legacy mitk::GLMapper for integration into the VTK rendering pipeline.
 *
 * Delegates VTK render calls to the wrapped GLMapper's Paint() method, enabling
 * legacy OpenGL mappers to participate in the modern VTK-based rendering pipeline.
 *
 * \sa VtkGLMapperWrapper, GLMapper
 */
class MITKLEGACYGL_EXPORT vtkGLMapperProp : public vtkProp
{
public:
  /** \brief Create a new instance. */
  static vtkGLMapperProp *New();
  vtkTypeMacro(vtkGLMapperProp, vtkProp);

  /** \brief Render the overlay by delegating to the wrapped GLMapper.
   * \param[in] viewport The viewport to render into.
   * \return 1 if rendering was performed, 0 otherwise.
   */
  int RenderOverlay(vtkViewport *viewport) override;

  /** \brief Volumetric geometry rendering pass (delegates to GLMapper).
   * \return 1 if rendering was performed, 0 otherwise.
   */
  int RenderVolumetricGeometry(vtkViewport *) override;

  /** \brief Translucent polygonal geometry rendering pass (delegates to GLMapper).
   * \return 1 if rendering was performed, 0 otherwise.
   */
  int RenderTranslucentPolygonalGeometry(vtkViewport *) override;

  /** \brief Opaque geometry rendering pass (delegates to GLMapper).
   * \return 1 if rendering was performed, 0 otherwise.
   */
  int RenderOpaqueGeometry(vtkViewport *) override;

  /** \brief Get the wrapped GLMapper.
   * \return The wrapped GLMapper, or \c nullptr if not set.
   */
  const mitk::GLMapper *GetWrappedGLMapper() const;

  /** \brief Set the GLMapper to wrap.
   * \param[in] glMapper The GLMapper to delegate rendering to.
   */
  void SetWrappedGLMapper(mitk::GLMapper *glMapper);

  /** \brief Set the base renderer context for the wrapped GLMapper.
   * \param[in] baseRenderer The MITK base renderer.
   */
  void SetBaseRenderer(mitk::BaseRenderer *baseRenderer);

protected:
  vtkGLMapperProp();
  ~vtkGLMapperProp() override;

  mitk::GLMapper *m_WrappedGLMapper;
  mitk::BaseRenderer *m_BaseRenderer;
};
#endif /* vtkGLMapperProp2_h */
