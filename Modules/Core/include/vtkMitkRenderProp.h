/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef VTKMITKRENDERPROP_H_HEADER_INCLUDED_C1C53723
#define VTKMITKRENDERPROP_H_HEADER_INCLUDED_C1C53723

#include "mitkVtkPropRenderer.h"
#include "vtkProp.h"

/*!
\brief vtkMitkRenderProp

The MITK rendering process is completely integrated into the VTK rendering pipeline.
The vtkMitkRenderProp is a custom vtkProp derived class, which implements the rendering interface between MITK and VTK.
It redirects VTK's various Render..Geometry() calls to mitk::VtkPropRenderer, which is responsible for rendering of mitk::DataNodes.

\sa rendering
\ingroup rendering
*/
class MITKCORE_EXPORT vtkMitkRenderProp : public vtkProp
{
public:
  static vtkMitkRenderProp *New();
  vtkTypeMacro(vtkMitkRenderProp, vtkProp);

  void SetPropRenderer(mitk::VtkPropRenderer::Pointer propRenderer);

  /**
   * \brief Store a vtkInformation object
   *
   * This method will forward the vtkInformation object
   * to the vtkProps of all mitk::VtkMapper
   */
  void SetPropertyKeys(vtkInformation *keys) override;

  int RenderOpaqueGeometry(vtkViewport *viewport) override;

  int RenderOverlay(vtkViewport *viewport) override;

  double *GetBounds() override;

  void ReleaseGraphicsResources(vtkWindow *window) override;

  /**
   * \brief Used by vtkPointPicker/vtkPicker.
   * This will query a list of all objects in MITK and provide every vtk based mapper to the picker.
   */
  void InitPathTraversal() override;

  /**
   * \brief Used by vtkPointPicker/vtkPicker.
   * This will query a list of all objects in MITK and provide every vtk based mapper to the picker.
   */
  vtkAssemblyPath *GetNextPath() override;

  int GetNumberOfPaths() override;

  int HasTranslucentPolygonalGeometry() override;
  int RenderTranslucentPolygonalGeometry(vtkViewport *) override;
  int RenderVolumetricGeometry(vtkViewport *) override;

protected:
  vtkMitkRenderProp();
  ~vtkMitkRenderProp() override;

  mitk::VtkPropRenderer::Pointer m_VtkPropRenderer;
};

#endif /* VTKMITKRENDERPROP_H_HEADER_INCLUDED_C1C53723 */
