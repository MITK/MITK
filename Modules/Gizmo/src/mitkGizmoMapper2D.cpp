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

#include "mitkGizmoMapper2D.h"

#include "mitkGizmo.h"

// MITK includes
#include <mitkVtkRepresentationProperty.h>
#include <mitkVtkInterpolationProperty.h>
#include <mitkLookupTableProperty.h>
#include <mitkVtkScalarModeProperty.h>
#include <mitkBaseRenderer.h>

// VTK includes
#include <vtkSphereSource.h>
#include <vtkCamera.h>
#include <vtkVectorOperators.h>

mitk::GizmoMapper2D::LocalStorage::LocalStorage()
{
    m_VtkPolyDataMapper = vtkSmartPointer<vtkPainterPolyDataMapper>::New();
    m_Actor = vtkSmartPointer<vtkActor>::New();
    m_Actor->SetMapper(m_VtkPolyDataMapper);
}

const mitk::Gizmo* mitk::GizmoMapper2D::GetInput()
{
    return static_cast<const Gizmo*>(GetDataNode()->GetData());
}

void mitk::GizmoMapper2D::ResetMapper(mitk::BaseRenderer* renderer)
{
    LocalStorage* ls = m_LSH.GetLocalStorage(renderer);
    ls->m_Actor->VisibilityOff();
}

void mitk::GizmoMapper2D::GenerateDataForRenderer(mitk::BaseRenderer* renderer)
{
    auto gizmo = GetInput();
    auto node = GetDataNode();

    LocalStorage* ls = m_LSH.GetLocalStorage(renderer);
    /*
    //check if something important has changed and we need to re-render
    if ( (ls->m_LastUpdateTime >= node->GetMTime())
        && (ls->m_LastUpdateTime >= gizmo->GetPipelineMTime())
        && (ls->m_LastUpdateTime >= renderer->GetCurrentWorldPlaneGeometryUpdateTime())
        && (ls->m_LastUpdateTime >= renderer->GetCurrentWorldPlaneGeometry()->GetMTime())
        && (ls->m_LastUpdateTime >= node->GetPropertyList()->GetMTime())
        && (ls->m_LastUpdateTime >= node->GetPropertyList(renderer)->GetMTime()) ) {
        return;
    }

    ls->m_LastUpdateTime.Modified();

    */

    auto camera = renderer->GetVtkRenderer()->GetActiveCamera();

    Point3D focus;
    camera->GetFocalPoint(focus.GetDataPointer());

    Point3D center = renderer->GetCurrentWorldGeometry2D()->ProjectPointOntoPlane(focus);

    double diagonal = std::min( renderer->GetSizeX(), renderer->GetSizeY() );

    auto sphereSource = vtkSmartPointer<vtkSphereSource>::New();
    sphereSource->SetCenter(center.GetDataPointer());
    sphereSource->SetRadius(0.01 * diagonal);
    sphereSource->SetThetaResolution(12);
    sphereSource->SetPhiResolution(12);
    sphereSource->Update();
    ls->m_VtkPolyDataMapper->SetInputData( sphereSource->GetOutput() );
}

