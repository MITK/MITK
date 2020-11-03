/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef vtkGLMapperProp_h
#define vtkGLMapperProp_h

#include "MitkLegacyGLExports.h"

#include "mitkGLMapper.h"
#include <vtkProp.h>
#include <vtkSmartPointer.h>

/**
 * @brief The vtkGLMapperProp class is a VtkProp, wrapping a GLMapper
 */
class MITKLEGACYGL_EXPORT vtkGLMapperProp : public vtkProp
{
public:
  static vtkGLMapperProp *New();
  vtkTypeMacro(vtkGLMapperProp, vtkProp);

  /**
   * @brief RenderOverlay Calls the render method of the actor and renders it.
   * @param viewport viewport of the renderwindow.
   * @return
   */
  int RenderOverlay(vtkViewport *viewport) override;
  int RenderVolumetricGeometry(vtkViewport *) override;
  int RenderTranslucentPolygonalGeometry(vtkViewport *) override;
  int RenderOpaqueGeometry(vtkViewport *) override;

  const mitk::GLMapper *GetWrappedGLMapper() const;
  void SetWrappedGLMapper(mitk::GLMapper *glMapper);

  void SetBaseRenderer(mitk::BaseRenderer *baseRenderer);

protected:
  vtkGLMapperProp();
  ~vtkGLMapperProp() override;

  mitk::GLMapper *m_WrappedGLMapper;
  mitk::BaseRenderer *m_BaseRenderer;
};
#endif /* vtkGLMapperProp2_h */
