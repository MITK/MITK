/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date: 2007-08-17 16:41:18 +0200 (Fr, 17 Aug 2007) $
Version:   $Revision: 11618 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "vtkMitkRenderProp.h"

#include <vtkObjectFactory.h>
#include <vtkLODProp3D.h>
#include <vtkPropAssembly.h>

#if ( ( VTK_MAJOR_VERSION >= 5 ) && ( VTK_MINOR_VERSION>=2)  )
#include "mitkBaseVtkMapper3D.h"
#endif

vtkStandardNewMacro(vtkMitkRenderProp);

vtkMitkRenderProp::vtkMitkRenderProp()
{
}
vtkMitkRenderProp::~vtkMitkRenderProp()
{
}

double *vtkMitkRenderProp::GetBounds()
{
  return const_cast<double*>(m_VtkPropRenderer->GetBounds());
}

void vtkMitkRenderProp::SetPropRenderer(mitk::VtkPropRenderer::Pointer propRenderer)
{
  this->m_VtkPropRenderer = propRenderer;
}

int vtkMitkRenderProp::RenderOpaqueGeometry(vtkViewport* /*viewport*/)
{
  return m_VtkPropRenderer->Render(mitk::VtkPropRenderer::Opaque); 
}

int vtkMitkRenderProp::RenderOverlay(vtkViewport* /*viewport*/)
{
  return m_VtkPropRenderer->Render(mitk::VtkPropRenderer::Overlay);
}

void vtkMitkRenderProp::ReleaseGraphicsResources(vtkWindow* window)
{
  m_VtkPropRenderer->ReleaseGraphicsResources(window);
}

void vtkMitkRenderProp::InitPathTraversal()
{
  m_VtkPropRenderer->InitPathTraversal();
}

vtkAssemblyPath* vtkMitkRenderProp::GetNextPath()
{
  return m_VtkPropRenderer->GetNextPath();
}
//BUG (#1551) added method depth peeling
#if ( ( VTK_MAJOR_VERSION >= 5 ) && ( VTK_MINOR_VERSION>=2)  )
int vtkMitkRenderProp::HasTranslucentPolygonalGeometry()
{
  typedef std::map<int,mitk::Mapper*> MappersMapType;
  MappersMapType mappersMap = m_VtkPropRenderer->GetMappersMap();
  for(MappersMapType::iterator it = mappersMap.begin(); it != mappersMap.end(); it++)
  {
    mitk::Mapper * mapper = (*it).second;
    mitk::BaseVtkMapper3D::Pointer vtkMapper = dynamic_cast<mitk::BaseVtkMapper3D*>(mapper);
    if(vtkMapper)
    {    
      // Due to VTK 5.2 bug, we need to initialize the Paths object in vtkPropAssembly
      // manually (see issue #8186 committed to VTK's Mantis issue tracker)
      // --> VTK bug resolved on 2008-12-01
      vtkPropAssembly *propAssembly = dynamic_cast< vtkPropAssembly * >(
        vtkMapper->GetVtkProp(m_VtkPropRenderer) );
      if ( propAssembly )
      {
        propAssembly->InitPathTraversal();
      }

      if (vtkMapper->GetVtkProp(m_VtkPropRenderer)->HasTranslucentPolygonalGeometry()==1)
        return 1;
    }

  }
  return 0;
}

int vtkMitkRenderProp::RenderTranslucentPolygonalGeometry( vtkViewport * )
{
  return m_VtkPropRenderer->Render(mitk::VtkPropRenderer::Translucent);
}
int vtkMitkRenderProp::RenderVolumetricGeometry( vtkViewport * )
{
  return m_VtkPropRenderer->Render(mitk::VtkPropRenderer::Volumetric);
}
#else
int vtkMitkRenderProp::RenderTranslucentGeometry(vtkViewport* /*viewport*/)
{
  return m_VtkPropRenderer->Render(mitk::VtkPropRenderer::Translucent);
}


#endif
