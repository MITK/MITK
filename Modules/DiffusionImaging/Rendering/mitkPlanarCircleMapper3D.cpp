/*
 *  mitkPlanarCircleMapper3D.cpp
 *  mitk-all
 *
 *  Created by HAL9000 on 2/15/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */



/*=========================================================================
 
 Program:   Medical Imaging & Interaction Toolkit
 Language:  C++
 Date:      $Date: 2009-05-12 19:56:03 +0200 (Di, 12 Mai 2009) $
 Version:   $Revision: 17179 $
 
 Copyright (c) German Cancer Research Center, Division of Medical and
 Biological Informatics. All rights reserved.
 See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.
 
 =========================================================================*/

#include "mitkPlanarCircleMapper3D.h"

#include <mitkProperties.h>
#include <mitkPlaneGeometry.h>
#include <mitkPlanarCircle.h>
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

#include <mitkVector.h>
//#include <mitkPlanarCircle.h>

#include <vtkOpenGLRenderer.h>
#include <vtkLookupTable.h>




//template<class TPixelType>
mitk::PlanarCircleMapper3D::PlanarCircleMapper3D() 
: m_vtkCircleList(NULL),
m_VtkCircleDataMapperGL(NULL)

{
  
  m_CircleAssembly = vtkPropAssembly::New();
  m_CircleActor = vtkOpenGLActor::New();
  m_polygonSource = vtkRegularPolygonSource::New();
  
  
}

//template<class TPixelType>
mitk::PlanarCircleMapper3D::~PlanarCircleMapper3D()
{
  //MITK_INFO << "FiberBundleMapper3D(destructor)";
  m_CircleActor->Delete();
  m_CircleAssembly->Delete();
  m_polygonSource->Delete();
  
  
}


const mitk::PlanarCircle* mitk::PlanarCircleMapper3D::GetInput()
{
  //MITK_INFO << "FiberBundleMapper3D GetInput()" ;
  
  return static_cast<const mitk::PlanarCircle * > ( GetData() );
}

/* 
 This method is called once the mapper gets new input, 
 for UI rotation or changes in colorcoding this method is NOT called 
 */
void mitk::PlanarCircleMapper3D::GenerateData()
{
  
//  bool is3DEnabled;
//  this->GetDataNode()->GetPropertyValue("PlanarCircle_3D",is3DEnabled);
//  if (!is3DEnabled) {
//    return;
//  }
  
  
  /* get my circle */
  mitk::PlanarCircle* PFCircle = dynamic_cast< mitk::PlanarCircle* > (this->GetData());
  
  try{ 
    
    
    mitk::Point3D V1 = PFCircle->GetWorldControlPoint(0); //centerPoint
    mitk::Point3D V2  = PFCircle->GetWorldControlPoint(1); //radiusPoint
    
    
    const mitk::Geometry2D* pfgeometry = PFCircle->GetGeometry2D();
    const mitk::PlaneGeometry* planeGeo = dynamic_cast<const mitk::PlaneGeometry*> (pfgeometry);
    mitk::Vector3D N = planeGeo->GetNormal();
    
    /*  calculate Radius 
     sqrt( (x1-x2)^2  (y1-y2)^2 (z1-z2)^2 )
     */ 
    
    double circleRadius = sqrt( pow( (double)(V1[0]-V2[0]), 2.0) + pow((double)(V1[1]-V2[1]), 2.0) +  pow( (double)(V1[2]-V2[2]), 2.0) );
    //MITK_INFO << "circleRadius: " << circleRadius;
    
    
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
    m_CircleActor->GetProperty()->SetOpacity(0.8);
    m_CircleAssembly->AddPart(m_CircleActor);
    
    this->GetDataNode()->SetColor(200.0,200.0,200.0);
    this->GetDataNode()->SetOpacity(0.8);
    
  } 
  catch(...) { 
//    MITK_INFO << "catch in PlanarCircleMapper3D..."; 
  } 
  
}




//template<class TPixelType>
void mitk::PlanarCircleMapper3D::GenerateDataForRenderer( mitk::BaseRenderer *renderer )
{
  // Update Circle in 3D Renderwindow
//  bool is3DEnabled;
//  this->GetDataNode()->GetPropertyValue("PlanarCircle_3D",is3DEnabled);
//  if (!is3DEnabled) {
//    return;
//  }
  
  try{
    mitk::PlanarCircle* PFCircle = dynamic_cast< mitk::PlanarCircle* > (this->GetData());
    mitk::Point3D V1 = PFCircle->GetWorldControlPoint(0); //centerPoint
    mitk::Point3D V2 = PFCircle->GetWorldControlPoint(1); //radiusPoint
    
    /*  calculate Radius 
     sqrt( (x1-x2)^2 + (y1-y2)^2 + (z1-z2)^2 )
     */ 
    
    double circleRadius = sqrt( pow( (double)(V1[0]-V2[0]), 2.0) + pow((double)(V1[1]-V2[1]), 2.0) +  pow( (double)(V1[2]-V2[2]), 2.0) );
    
    m_polygonSource->SetCenter((double)V1[0],(double)V1[1],(double)V1[2]);
    m_polygonSource->SetRadius(circleRadius);
    m_polygonSource->Update();
    
    
    float polyOpaq;
    this->GetDataNode()->GetOpacity(polyOpaq, NULL);
    m_CircleActor->GetProperty()->SetOpacity((double) polyOpaq);
    
    float temprgb[3];
    this->GetDataNode()->GetColor( temprgb, NULL );
    double trgb[3] = { (double) temprgb[0], (double) temprgb[1], (double) temprgb[2] };
    m_CircleActor->GetProperty()->SetColor(trgb);
    
    
  }
  catch (...) {
//    MITK_INFO << "catch generateData(baseRenderer)";
  }
}

//template<class TPixelType>
void mitk::PlanarCircleMapper3D::SetDefaultProperties(mitk::DataNode* node, mitk::BaseRenderer* renderer, bool overwrite)
{
 // node->AddProperty( "PlanarCircle_3D", mitk::BoolProperty::New( true ), renderer, overwrite  );
  Superclass::SetDefaultProperties(node, renderer, overwrite); 
}

vtkProp* mitk::PlanarCircleMapper3D::GetVtkProp(mitk::BaseRenderer *renderer)
{
  
  //MITK_INFO << "FiberBundleMapper3D GetVtkProp(renderer)";
  return m_CircleAssembly;
  
}

void mitk::PlanarCircleMapper3D::ApplyProperties(mitk::BaseRenderer* renderer)
{
}

void mitk::PlanarCircleMapper3D::UpdateVtkObjects()
{
  // MITK_INFO << "FiberBundleMapper3D UpdateVtkObjects()";
  
  
}

void mitk::PlanarCircleMapper3D::SetVtkMapperImmediateModeRendering(vtkMapper *)
{
  
  
  
}



