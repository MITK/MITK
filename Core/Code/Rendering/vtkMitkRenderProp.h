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


#ifndef VTKMITKRENDERPROP_H_HEADER_INCLUDED_C1C53723
#define VTKMITKRENDERPROP_H_HEADER_INCLUDED_C1C53723

#include "vtkProp.h"
#include "mitkVtkPropRenderer.h"


/*!
\brief vtkMitkRenderProp

The MITK rendering process is completely integrated into the VTK rendering pipeline.
The vtkMitkRenderProp is a custom vtkProp derived class, which implements the rendering interface between MITK and VTK. It redirects render() calls to the VtkPropRenderer, which is responsible for rendering of the datatreenodes.

\sa rendering
\ingroup rendering
*/
class MITK_CORE_EXPORT vtkMitkRenderProp : public vtkProp
{
  public:
    static vtkMitkRenderProp *New();
    vtkTypeMacro(vtkMitkRenderProp,vtkProp);

    void SetPropRenderer(mitk::VtkPropRenderer::Pointer propRenderer);

    int RenderOpaqueGeometry(vtkViewport* viewport);

    int RenderOverlay(vtkViewport* viewport);

    double *GetBounds();

    void ReleaseGraphicsResources(vtkWindow* window);

    /**
     * \brief Used by vtkPointPicker/vtkPicker.
     * This will query a list of all objects in MITK and provide every vtk based mapper to the picker.
     */
    virtual void InitPathTraversal();

    /**
     * \brief Used by vtkPointPicker/vtkPicker.
     * This will query a list of all objects in MITK and provide every vtk based mapper to the picker.
     */
    virtual vtkAssemblyPath* GetNextPath();

    virtual int GetNumberOfPaths();

    //BUG (#1551) added method for depth peeling support
    virtual int HasTranslucentPolygonalGeometry();
    virtual int RenderTranslucentPolygonalGeometry( vtkViewport *);
    virtual int RenderVolumetricGeometry( vtkViewport *);

  protected:
    vtkMitkRenderProp();
    ~vtkMitkRenderProp();

    mitk::VtkPropRenderer::Pointer m_VtkPropRenderer;
};


#endif /* VTKMITKRENDERPROP_H_HEADER_INCLUDED_C1C53723 */
