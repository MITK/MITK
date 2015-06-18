/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef vtkGLMapperProp_h
#define vtkGLMapperProp_h

#include "MitkLegacyGLExports.h"

#include <vtkSmartPointer.h>
#include <vtkProp.h>
#include "mitkGLMapper.h"

/**
 * @brief The vtkGLMapperProp class is a VtkProp, wrapping a GLMapper
 */
class MITKLEGACYGL_EXPORT vtkGLMapperProp : public vtkProp
{
  public:
    static vtkGLMapperProp* New();
    vtkTypeMacro(vtkGLMapperProp,vtkProp);

    /**
     * @brief RenderOverlay Calls the render method of the actor and renders it.
     * @param viewport viewport of the renderwindow.
     * @return
     */
    int RenderOverlay(vtkViewport* viewport) override;
    int RenderVolumetricGeometry(vtkViewport *) override;
    int RenderTranslucentPolygonalGeometry(vtkViewport *) override;
    int RenderOpaqueGeometry(vtkViewport *) override;

    const mitk::GLMapper *GetWrappedGLMapper() const;
    void SetWrappedGLMapper(mitk::GLMapper *glMapper);

    void SetBaseRenderer(mitk::BaseRenderer* baseRenderer);
protected:

    vtkGLMapperProp();
    virtual ~vtkGLMapperProp();

    mitk::GLMapper* m_WrappedGLMapper;
    mitk::BaseRenderer* m_BaseRenderer;

};
#endif /* vtkGLMapperProp2_h */
