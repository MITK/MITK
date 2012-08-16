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

#include "mitkPlanarCircleMapper3D.h"

#include <mitkProperties.h>
#include <mitkPlaneGeometry.h>
#include <mitkPlanarCircle.h>
#include <mitkVector.h>

#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkDoubleArray.h>
#include <vtkPoints.h>
#include <vtkPolyLine.h>
#include <vtkPolyData.h>
#include <vtkPointData.h>
#include <vtkTubeFilter.h>
#include <vtkCamera.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkVertex.h>
#include <vtkOpenGLRenderer.h>
#include <vtkLookupTable.h>


mitk::PlanarCircleMapper3D::PlanarCircleMapper3D()
  : m_vtkCircleList(NULL),
    m_VtkCircleDataMapperGL(NULL)
{
  m_CircleAssembly = vtkPropAssembly::New();
  m_CircleActor = vtkOpenGLActor::New();
  m_polygonSource = vtkRegularPolygonSource::New();
}

mitk::PlanarCircleMapper3D::~PlanarCircleMapper3D()
{

}

const mitk::PlanarCircle* mitk::PlanarCircleMapper3D::GetInput()
{
  return static_cast<const mitk::PlanarCircle * > ( GetData() );
}

/*
 This method is called once the mapper gets new input,
 for UI rotation or changes in colorcoding this method is NOT called
 */
void mitk::PlanarCircleMapper3D::GenerateData()
{
  mitk::PlanarCircle* PFCircle = dynamic_cast< mitk::PlanarCircle* > (this->GetData());

  try
  {
    mitk::Point3D V1 = PFCircle->GetWorldControlPoint(0); //centerPoint
    mitk::Point3D V2  = PFCircle->GetWorldControlPoint(1); //radiusPoint


    const mitk::Geometry2D* pfgeometry = PFCircle->GetGeometry2D();
    const mitk::PlaneGeometry* planeGeo = dynamic_cast<const mitk::PlaneGeometry*> (pfgeometry);
    mitk::Vector3D N = planeGeo->GetNormal();

    double circleRadius = V1.EuclideanDistanceTo(V2);

    m_polygonSource->SetNormal((double)N[0],(double)N[1],(double)N[2]);
    m_polygonSource->SetNumberOfSides(50);
    m_polygonSource->SetRadius(circleRadius);
    m_polygonSource->SetCenter(V1[0],V1[1],V1[2]);
    m_polygonSource->Update();

    // Visualize
    m_VtkCircleDataMapperGL =   vtkOpenGLPolyDataMapper::New();
    m_VtkCircleDataMapperGL->SetInputConnection(m_polygonSource->GetOutputPort());

    m_CircleActor = vtkOpenGLActor::New();
    m_CircleActor->SetMapper(m_VtkCircleDataMapperGL);
    m_CircleAssembly->AddPart(m_CircleActor);


    // Visualize normal if required
    bool showNormal;
    bool hasNormalProp = this->GetDataNode()->GetBoolProperty("shownormal",showNormal);


    if(hasNormalProp && showNormal)
    {

      m_NormalActor = vtkActor::New();
      m_NormalMapper = vtkPolyDataMapper::New();



      vtkSmartPointer<vtkPoints> points = vtkPoints::New();

      double p0[3] = {V1[0],V1[1],V1[2]};
      double p1[3] = {V1[0]+(double)N[0],
                      V1[1]+(double)N[1],
                      V1[2]+(double)N[2]};



      points->InsertNextPoint(p0);
      points->InsertNextPoint(p1);

      vtkSmartPointer<vtkPolyLine> polyLine = vtkPolyLine::New();


      polyLine->GetPointIds()->SetNumberOfIds(2);
      for(unsigned int i = 0; i < 2; i++)
      {
        polyLine->GetPointIds()->SetId(i,i);
      }




      // Create a cell array to store the lines in and add the lines to it
      vtkSmartPointer<vtkCellArray> cells =
        vtkSmartPointer<vtkCellArray>::New();
      cells->InsertNextCell(polyLine);


      // Create a polydata to store everything in
      vtkSmartPointer<vtkPolyData> polyData =
        vtkSmartPointer<vtkPolyData>::New();


      // Add the points to the dataset
      polyData->SetPoints(points);

      // Add the lines to the dataset
      polyData->SetLines(cells);


      m_NormalMapper->SetInput(polyData);
      m_NormalActor->SetMapper(m_NormalMapper);
      m_CircleAssembly->AddPart(m_NormalActor);


    }


  }
  catch(...)
  {

  }
}

void mitk::PlanarCircleMapper3D::GenerateDataForRenderer( mitk::BaseRenderer *renderer )
{
  try
  {
    mitk::PlanarCircle* PFCircle = dynamic_cast< mitk::PlanarCircle* > (this->GetData());
    mitk::Point3D V1 = PFCircle->GetWorldControlPoint(0); //centerPoint
    mitk::Point3D V2 = PFCircle->GetWorldControlPoint(1); //radiusPoint

    double circleRadius = V1.EuclideanDistanceTo(V2);

    m_polygonSource->SetCenter((double)V1[0],(double)V1[1],(double)V1[2]);
    m_polygonSource->SetRadius(circleRadius);
    m_polygonSource->Update();

    float polyOpaq;
    this->GetDataNode()->GetOpacity(polyOpaq, NULL);
    if (polyOpaq>=1) // hack to avoid wrong color if opacity==1
      polyOpaq = 0.999;
    m_CircleActor->GetProperty()->SetOpacity((double) polyOpaq);

    float temprgb[3];
    this->GetDataNode()->GetColor( temprgb, NULL );
    double trgb[3] = { (double) temprgb[0], (double) temprgb[1], (double) temprgb[2] };
    m_CircleActor->GetProperty()->SetColor(trgb);
  }
  catch (...)
  {

  }
}

void mitk::PlanarCircleMapper3D::SetDefaultProperties(mitk::DataNode* node, mitk::BaseRenderer* renderer, bool overwrite)
{
  Superclass::SetDefaultProperties(node, renderer, overwrite);
}

vtkProp* mitk::PlanarCircleMapper3D::GetVtkProp(mitk::BaseRenderer *renderer)
{
  return m_CircleAssembly;
}

void mitk::PlanarCircleMapper3D::ApplyProperties(mitk::BaseRenderer* renderer)
{
}

void mitk::PlanarCircleMapper3D::UpdateVtkObjects()
{
}

void mitk::PlanarCircleMapper3D::SetVtkMapperImmediateModeRendering(vtkMapper *)
{
}
