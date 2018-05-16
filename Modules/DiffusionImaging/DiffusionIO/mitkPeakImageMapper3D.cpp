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

#include "mitkPeakImageMapper3D.h"
#include "mitkBaseRenderer.h"
#include "mitkDataNode.h"
#include <vtkActor.h>
#include <vtkOpenGLPolyDataMapper.h>
#include <vtkOpenGLHelper.h>
#include <vtkShaderProgram.h>
#include <vtkPlane.h>
#include <vtkPolyData.h>
#include <vtkPointData.h>
#include <vtkProperty.h>
#include <vtkLookupTable.h>
#include <vtkPoints.h>
#include <vtkCamera.h>
#include <vtkPolyLine.h>
#include <vtkRenderer.h>
#include <vtkCellArray.h>
#include <vtkMatrix4x4.h>
#include <vtkTubeFilter.h>
#include <mitkPlaneGeometry.h>
#include <mitkSliceNavigationController.h>
#include <mitkCoreServices.h>
#include <mitkClippingProperty.h>

mitk::PeakImageMapper3D::PeakImageMapper3D()
{
  m_lut = vtkSmartPointer<vtkLookupTable>::New();
  m_lut->Build();
}

mitk::PeakImageMapper3D::~PeakImageMapper3D()
{
}


mitk::PeakImage* mitk::PeakImageMapper3D::GetInput()
{
  return dynamic_cast< mitk::PeakImage * > ( GetDataNode()->GetData() );
}

void mitk::PeakImageMapper3D::UpdateVtkTransform(mitk::BaseRenderer *)
{
  // don't apply transform since the peak polydata is already in world coordinates.
  return;
}

void mitk::PeakImageMapper3D::Update(mitk::BaseRenderer * renderer)
{
  mitk::DataNode* node = this->GetDataNode();
  if (node == nullptr)
    return;

  bool visible = true;
  node->GetVisibility(visible, renderer, "visible");
  if ( !visible )
    return;

  this->GenerateDataForRenderer( renderer );
}

// vtkActors and Mappers are feeded here
void mitk::PeakImageMapper3D::GenerateDataForRenderer(mitk::BaseRenderer *renderer)
{
  //the handler of local storage gets feeded in this method with requested data for related renderwindow
  LocalStorage *localStorage = m_LocalStorageHandler.GetLocalStorage(renderer);

  bool enabled = false;
  this->GetDataNode()->GetBoolProperty("Enable3DPeaks",enabled);
  if (!enabled)
  {
    vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New();
    localStorage->m_Mapper->SetInputData(polyData);
    localStorage->m_Actor->SetMapper(localStorage->m_Mapper);
    localStorage->m_Assembly->AddPart(localStorage->m_Actor);
    return;
  }

  mitk::PeakImage* peakImage = this->GetInput();
  vtkSmartPointer<vtkPolyData> polyData = peakImage->GetPolyData();
  if (polyData == nullptr)
    return;

  float linewidth = 1.0;
  this->GetDataNode()->GetFloatProperty("shape.linewidth",linewidth);

  localStorage->m_Mapper->SetInputData(polyData);
  localStorage->m_Mapper->SelectColorArray("FIBER_COLORS");
  localStorage->m_Mapper->ScalarVisibilityOn();
  localStorage->m_Mapper->SetScalarModeToUsePointFieldData();
  localStorage->m_Actor->SetMapper(localStorage->m_Mapper);
  localStorage->m_Mapper->SetLookupTable(m_lut);
  localStorage->m_Actor->GetProperty()->SetLineWidth(linewidth);
  localStorage->m_Assembly->AddPart(localStorage->m_Actor);

  const DataNode* node = this->GetDataNode();
  mitk::ClippingProperty* prop = dynamic_cast<mitk::ClippingProperty*>(node->GetProperty("3DClipping"));

  mitk::Image* image = dynamic_cast<mitk::Image*>(node->GetData());
  localStorage->m_Mapper->RemoveAllClippingPlanes();
  mitk::Vector3D spacing = image->GetGeometry()->GetSpacing();
  float clipping_plane_thickness = 1;
  if(spacing[0]<spacing[1] && spacing[0]<spacing[2])
    clipping_plane_thickness = spacing[0];
  else if (spacing[1] < spacing[2])
    clipping_plane_thickness = spacing[1];
  else
    clipping_plane_thickness = spacing[2];
  clipping_plane_thickness /= 2.0;

  mitk::Vector3D plane_normal = prop->GetNormal();
  if (plane_normal.GetNorm()>0.0)
  {
    plane_normal.Normalize();
    mitk::Point3D plane_origin = prop->GetOrigin();

    double vnormal[3];
    double vp1[3];
    double vp2[3];

    vp1[0] = plane_origin[0] + plane_normal[0] * clipping_plane_thickness;
    vp1[1] = plane_origin[1] + plane_normal[1] * clipping_plane_thickness;
    vp1[2] = plane_origin[2] + plane_normal[2] * clipping_plane_thickness;

    vp2[0] = plane_origin[0] - plane_normal[0] * clipping_plane_thickness;
    vp2[1] = plane_origin[1] - plane_normal[1] * clipping_plane_thickness;
    vp2[2] = plane_origin[2] - plane_normal[2] * clipping_plane_thickness;

    {
      vnormal[0] = vp2[0] - vp1[0];
      vnormal[1] = vp2[1] - vp1[1];
      vnormal[2] = vp2[2] - vp1[2];

      vtkSmartPointer<vtkPlane> plane = vtkSmartPointer<vtkPlane>::New();
      plane->SetOrigin(vp1);
      plane->SetNormal(vnormal);
      localStorage->m_Mapper->AddClippingPlane(plane);
    }
    {
      vnormal[0] = vp1[0] - vp2[0];
      vnormal[1] = vp1[1] - vp2[1];
      vnormal[2] = vp1[2] - vp2[2];

      vtkSmartPointer<vtkPlane> plane = vtkSmartPointer<vtkPlane>::New();
      plane->SetOrigin(vp2);
      plane->SetNormal(vnormal);
      localStorage->m_Mapper->AddClippingPlane(plane);
    }
  }

  // We have been modified => save this for next Update()
  localStorage->m_LastUpdateTime.Modified();
}


vtkProp* mitk::PeakImageMapper3D::GetVtkProp(mitk::BaseRenderer *renderer)
{
  return m_LocalStorageHandler.GetLocalStorage(renderer)->m_Assembly;
}


void mitk::PeakImageMapper3D::SetDefaultProperties(mitk::DataNode* node, mitk::BaseRenderer* renderer, bool overwrite)
{
  Superclass::SetDefaultProperties(node, renderer, overwrite);

  mitk::Vector3D plane_vec; plane_vec.Fill(0.0);
  mitk::Point3D plane_origin; plane_origin.Fill(0.0);
  node->AddProperty( "3DClipping", mitk::ClippingProperty::New( plane_origin, plane_vec ), renderer, overwrite );
  node->AddProperty( "3DClippingPlaneId", mitk::IntProperty::New(-1), renderer, overwrite );
  node->AddProperty( "3DClippingPlaneFlip", mitk::BoolProperty::New( false ), renderer, overwrite );
  node->AddProperty( "Enable3DPeaks", mitk::BoolProperty::New( false ), renderer, overwrite );
}


mitk::PeakImageMapper3D::LocalStorage::LocalStorage()
{
  m_Actor = vtkSmartPointer<vtkActor>::New();
  m_Mapper = vtkSmartPointer<MITKPeakImageMapper3D_POLYDATAMAPPER>::New();
  m_Assembly = vtkSmartPointer<vtkPropAssembly>::New();
}
