/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "vtkMitkRenderProp.h"

#include <vtkObjectFactory.h>
#include <vtkPropAssembly.h>
#include <vtkInformation.h>

#include "mitkVtkMapper.h"

vtkStandardNewMacro(vtkMitkRenderProp);

vtkMitkRenderProp::vtkMitkRenderProp()
{
}
vtkMitkRenderProp::~vtkMitkRenderProp()
{
}

double *vtkMitkRenderProp::GetBounds()
{
  return const_cast<double *>(m_VtkPropRenderer->GetBounds());
}

void vtkMitkRenderProp::SetPropRenderer(mitk::VtkPropRenderer::Pointer propRenderer)
{
  this->m_VtkPropRenderer = propRenderer;
}

void vtkMitkRenderProp::SetPropertyKeys(vtkInformation *keys)
{
  Superclass::SetPropertyKeys(keys);

  if (nullptr != m_VtkPropRenderer)
    m_VtkPropRenderer->SetPropertyKeys(keys);
}

int vtkMitkRenderProp::RenderOpaqueGeometry(vtkViewport * /*viewport*/)
{
  return m_VtkPropRenderer->Render(mitk::VtkPropRenderer::Opaque);
}

int vtkMitkRenderProp::RenderOverlay(vtkViewport * /*viewport*/)
{
  return m_VtkPropRenderer->Render(mitk::VtkPropRenderer::Overlay);
}

void vtkMitkRenderProp::ReleaseGraphicsResources(vtkWindow *window)
{
  m_VtkPropRenderer->ReleaseGraphicsResources(window);
}

void vtkMitkRenderProp::InitPathTraversal()
{
  m_VtkPropRenderer->InitPathTraversal();
}

vtkAssemblyPath *vtkMitkRenderProp::GetNextPath()
{
  return m_VtkPropRenderer->GetNextPath();
}

int vtkMitkRenderProp::GetNumberOfPaths()
{
  return m_VtkPropRenderer->GetNumberOfPaths();
}

int vtkMitkRenderProp::HasTranslucentPolygonalGeometry()
{
  for (const auto &mapEntry : m_VtkPropRenderer->GetMappersMap())
  {
    auto vtkMapper = dynamic_cast<mitk::VtkMapper*>(mapEntry.second);

    if (nullptr != vtkMapper)
    {
      // Due to VTK 5.2 bug, we need to initialize the Paths object in vtkPropAssembly
      // manually (see issue #8186 committed to VTK's Mantis issue tracker)
      // --> VTK bug resolved on 2008-12-01
      auto propAssembly = dynamic_cast<vtkPropAssembly *>(vtkMapper->GetVtkProp(m_VtkPropRenderer));

      if (nullptr != propAssembly)
        propAssembly->InitPathTraversal();

      if (1 == vtkMapper->GetVtkProp(m_VtkPropRenderer)->HasTranslucentPolygonalGeometry())
        return 1;
    }
  }

  return 0;
}

int vtkMitkRenderProp::RenderTranslucentPolygonalGeometry(vtkViewport *)
{
  return m_VtkPropRenderer->Render(mitk::VtkPropRenderer::Translucent);
}
int vtkMitkRenderProp::RenderVolumetricGeometry(vtkViewport *)
{
  return m_VtkPropRenderer->Render(mitk::VtkPropRenderer::Volumetric);
}
