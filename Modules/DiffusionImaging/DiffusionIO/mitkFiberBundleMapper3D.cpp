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

#include "mitkFiberBundleMapper3D.h"
#include <mitkProperties.h>

#include <vtkPropAssembly.h>
#include <vtkPointData.h>
#include <vtkProperty.h>
#include <vtkCellArray.h>
#include <vtkDepthSortPolyData.h>
#include <vtkCamera.h>
#include <vtkTubeFilter.h>
#include <vtkRibbonFilter.h>
#include <vtkLookupTable.h>
#include <vtkOpenGLHelper.h>
#include <vtkShaderProgram.h>
#include <mitkVectorProperty.h>
#include <vtkPlane.h>
#include <mitkClippingProperty.h>

mitk::FiberBundleMapper3D::FiberBundleMapper3D()
  : m_TubeRadius(0.0)
  , m_TubeSides(15)
  , m_LineWidth(1)
{
  m_lut = vtkSmartPointer<vtkLookupTable>::New();
  m_lut->Build();
}


mitk::FiberBundleMapper3D::~FiberBundleMapper3D()
{

}


const mitk::FiberBundle* mitk::FiberBundleMapper3D::GetInput()
{
  return static_cast<const mitk::FiberBundle * > ( GetDataNode()->GetData() );
}


/*
 This method is called once the mapper gets new input,
 for UI rotation or changes in colorcoding this method is NOT called
 */
void mitk::FiberBundleMapper3D::InternalGenerateData(mitk::BaseRenderer *renderer)
{
  m_FiberPolyData->GetPointData()->AddArray(m_FiberBundle->GetFiberColors());
  LocalStorage3D *localStorage = m_LocalStorageHandler.GetLocalStorage(renderer);

  if (m_TubeRadius>0.0f)
  {
    vtkSmartPointer<vtkTubeFilter> tubeFilter = vtkSmartPointer<vtkTubeFilter>::New();
    tubeFilter->SetInputData(m_FiberPolyData);
    tubeFilter->SetNumberOfSides(m_TubeSides);
    tubeFilter->SetRadius(m_TubeRadius);
    tubeFilter->Update();
    m_FiberPolyData = tubeFilter->GetOutput();
  }
  else if (m_RibbonWidth>0.0f)
  {
    vtkSmartPointer<vtkRibbonFilter> tubeFilter = vtkSmartPointer<vtkRibbonFilter>::New();
    tubeFilter->SetInputData(m_FiberPolyData);
    tubeFilter->SetWidth(m_RibbonWidth);
    tubeFilter->Update();
    m_FiberPolyData = tubeFilter->GetOutput();
  }

//  if (tmpopa<1)
//  {
//    vtkSmartPointer<vtkDepthSortPolyData> depthSort = vtkSmartPointer<vtkDepthSortPolyData>::New();
//    depthSort->SetInputData( m_FiberPolyData );
//    depthSort->SetCamera( renderer->GetVtkRenderer()->GetActiveCamera() );
//    depthSort->SetDirectionToBackToFront();
//    depthSort->Update();
//    localStorage->m_FiberMapper->SetInputConnection(depthSort->GetOutputPort());

//  }
//  else
//  {
    localStorage->m_FiberMapper->SetInputData(m_FiberPolyData);
//  }

  localStorage->m_FiberMapper->SelectColorArray("FIBER_COLORS");
  localStorage->m_FiberMapper->ScalarVisibilityOn();
  localStorage->m_FiberMapper->SetScalarModeToUsePointFieldData();
  localStorage->m_FiberActor->SetMapper(localStorage->m_FiberMapper);
  localStorage->m_FiberMapper->SetLookupTable(m_lut);
  localStorage->m_FiberActor->GetProperty()->SetLineWidth(m_LineWidth);
  localStorage->m_FiberAssembly->AddPart(localStorage->m_FiberActor);

  DataNode* node = this->GetDataNode();
  mitk::ClippingProperty* prop = dynamic_cast<mitk::ClippingProperty*>(node->GetProperty("3DClipping"));
  if (prop==nullptr)
  {
    SetDefaultProperties(node, nullptr, false);
    prop = dynamic_cast<mitk::ClippingProperty*>(node->GetProperty("3DClipping"));
  }

  mitk::Vector3D plane_normal = prop->GetNormal();
  mitk::Point3D plane_origin = prop->GetOrigin();
  bool flip;
  node->GetBoolProperty("3DClippingPlaneFlip",flip);

  if (flip)
    plane_normal *= -1;

  vtkSmartPointer<vtkPlane> plane = vtkSmartPointer<vtkPlane>::New();
  double vp[3], vnormal[3];
  vp[0] = plane_origin[0]; vp[1] = plane_origin[1]; vp[2] = plane_origin[2];
  vnormal[0] = plane_normal[0]; vnormal[1] = plane_normal[1]; vnormal[2] = plane_normal[2];
  plane->SetOrigin(vp);
  plane->SetNormal(vnormal);

  localStorage->m_FiberMapper->RemoveAllClippingPlanes();
  if (plane_normal.GetNorm() > 0.0)
    localStorage->m_FiberMapper->AddClippingPlane(plane);

  localStorage->m_LastUpdateTime.Modified();
}



void mitk::FiberBundleMapper3D::GenerateDataForRenderer( mitk::BaseRenderer *renderer )
{
  bool visible = true;
  GetDataNode()->GetVisibility(visible, renderer, "visible");
  if ( !visible )
    return;

  const DataNode* node = this->GetDataNode();
  LocalStorage3D* localStorage = m_LocalStorageHandler.GetLocalStorage(renderer);

  m_FiberBundle = dynamic_cast<mitk::FiberBundle*>(node->GetData());
  m_FiberPolyData = m_FiberBundle->GetFiberPolyData();

  // did any rendering properties change?
  float tubeRadius = 0;
  node->GetFloatProperty("shape.tuberadius", tubeRadius);
  if (m_TubeRadius!=tubeRadius)
  {
    m_TubeRadius = tubeRadius;
    m_FiberBundle->RequestUpdate3D();
  }

  int tubeSides = 0;
  node->GetIntProperty("shape.tubesides", tubeSides);
  if (m_TubeSides!=tubeSides)
  {
    m_TubeSides = tubeSides;
    m_FiberBundle->RequestUpdate3D();
  }

  int lineWidth = 0;
  node->GetIntProperty("shape.linewidth", lineWidth);
  if (m_LineWidth!=lineWidth)
  {
    m_LineWidth = lineWidth;
    m_FiberBundle->RequestUpdate3D();
  }

  float ribbonWidth = 0;
  node->GetFloatProperty("shape.ribbonwidth", ribbonWidth);
  if (m_RibbonWidth!=ribbonWidth)
  {
    m_RibbonWidth = ribbonWidth;
    m_FiberBundle->RequestUpdate3D();
  }

  float opacity;
  this->GetDataNode()->GetOpacity(opacity, nullptr);
  vtkProperty *property = localStorage->m_FiberActor->GetProperty();

  float v = 1;
  node->GetFloatProperty("light.ambient", v);
  property->SetAmbient(v);

  node->GetFloatProperty("light.diffuse", v);
  property->SetDiffuse(v);

  node->GetFloatProperty("light.specular", v);
  property->SetSpecular(v);

  node->GetFloatProperty("light.specularpower", v);
  property->SetSpecularPower(v);

  property->SetLighting(true);
  property->SetOpacity(opacity);

  if (localStorage->m_LastUpdateTime>=m_FiberBundle->GetUpdateTime3D())
    return;

  // Calculate time step of the input data for the specified renderer (integer value)
  // this method is implemented in mitkMapper
  this->CalculateTimeStep( renderer );
  this->InternalGenerateData(renderer);
}

void mitk::FiberBundleMapper3D::UpdateShaderParameter(mitk::BaseRenderer * )
{
  // see new vtkShaderCallback3D
}

void mitk::FiberBundleMapper3D::SetDefaultProperties(mitk::DataNode* node, mitk::BaseRenderer* renderer, bool overwrite)
{
  Superclass::SetDefaultProperties(node, renderer, overwrite);

  mitk::Vector3D plane_vec; plane_vec.Fill(0.0);
  mitk::Point3D plane_origin; plane_origin.Fill(0.0);
  node->AddProperty( "3DClipping", mitk::ClippingProperty::New( plane_origin, plane_vec ), renderer, overwrite );
  node->AddProperty( "3DClippingPlaneId", mitk::IntProperty::New(-1), renderer, overwrite );
  node->AddProperty( "3DClippingPlaneFlip", mitk::BoolProperty::New( false ), renderer, overwrite );

  node->AddProperty( "opacity", mitk::FloatProperty::New( 1.0 ), renderer, overwrite);
  node->AddProperty( "color", mitk::ColorProperty::New(1.0,1.0,1.0), renderer, overwrite);
  node->AddProperty( "pickable", mitk::BoolProperty::New( true ), renderer, overwrite);

  node->AddProperty( "shape.linewidth", mitk::IntProperty::New( 1 ), renderer, overwrite );
  node->AddProperty( "shape.tuberadius",mitk::FloatProperty::New( 0.0 ), renderer, overwrite);
  node->AddProperty( "shape.tubesides",mitk::IntProperty::New( 15 ), renderer, overwrite);
  node->AddProperty( "shape.ribbonwidth", mitk::FloatProperty::New( 0.0 ), renderer, overwrite);

  node->AddProperty( "light.ambient", mitk::FloatProperty::New( 0.05 ), renderer, overwrite);
  node->AddProperty( "light.diffuse", mitk::FloatProperty::New( 0.9 ), renderer, overwrite);
  node->AddProperty( "light.specular", mitk::FloatProperty::New( 1.0 ), renderer, overwrite);

  node->AddProperty( "light.specularpower", mitk::FloatProperty::New( 16.0 ), renderer, overwrite);
  node->AddProperty( "light.ambientcolor", mitk::ColorProperty::New(1,1,1), renderer, overwrite);
  node->AddProperty( "light.diffusecolor", mitk::ColorProperty::New(1,1,1), renderer, overwrite);
  node->AddProperty( "light.specularcolor", mitk::ColorProperty::New(1,1,1), renderer, overwrite);
}

vtkProp* mitk::FiberBundleMapper3D::GetVtkProp(mitk::BaseRenderer *renderer)
{
  return m_LocalStorageHandler.GetLocalStorage(renderer)->m_FiberAssembly;
}

mitk::FiberBundleMapper3D::LocalStorage3D::LocalStorage3D()
{
  m_FiberActor = vtkSmartPointer<vtkActor>::New();
  m_FiberMapper = vtkSmartPointer<vtkOpenGLPolyDataMapper>::New();
  m_FiberAssembly = vtkSmartPointer<vtkPropAssembly>::New();
}

