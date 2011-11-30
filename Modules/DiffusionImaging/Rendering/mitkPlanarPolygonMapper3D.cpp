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

#include "mitkPlanarPolygonMapper3D.h"
#include <mitkProperties.h>
#include <mitkPlaneGeometry.h>

#include <vtkPointData.h>
#include <vtkProperty.h>



//template<class TPixelType>
mitk::PlanarPolygonMapper3D::PlanarPolygonMapper3D() 
: m_points(vtkPoints::New())
, m_polygon(vtkPolygon::New())
, m_polygonPolyData(vtkPolyData::New())
, m_polygonsCell(vtkCellArray::New())
, m_VtkPolygonDataMapperGL(vtkOpenGLPolyDataMapper::New())
, m_PolygonActor(vtkOpenGLActor::New())
, m_PolygonAssembly(vtkPropAssembly::New())
{
  
}

//template<class TPixelType>
mitk::PlanarPolygonMapper3D::~PlanarPolygonMapper3D()
{
  //MITK_INFO << "FiberBundleMapper3D(destructor)";
  m_points->Delete();
  m_polygon->Delete();
  m_polygonPolyData->Delete();
  m_polygonsCell->Delete();
  m_VtkPolygonDataMapperGL->Delete();
  m_PolygonActor->Delete();
  m_PolygonAssembly->Delete();
  
}


const mitk::PlanarPolygon* mitk::PlanarPolygonMapper3D::GetInput()
{
  //MITK_INFO << "FiberBundleMapper3D GetInput()" ;
  
  return static_cast<const mitk::PlanarPolygon * > ( GetData() );
}

/* 
 This method is called once the mapper gets new input, 
 for UI rotation or changes in colorcoding this method is NOT called 
 */


/* */
void mitk::PlanarPolygonMapper3D::GenerateData()
{
  
  
  // bool enableIn3D;
  //check if property exists
  //return if circle should not be painted in 3d View
  //  bool is3DEnabled;
  //  this->GetDataNode()->GetPropertyValue("PlanarPolygon_3D",is3DEnabled);
  //  if (!is3DEnabled) {
  //    return;
  //  }
  
  
  try{ 
    /* get my polygon */
    mitk::PlanarPolygon* PFPolygon = dynamic_cast< mitk::PlanarPolygon* > (this->GetData());
    
    //get the control points from pf and insert them to vtkPolygon
    unsigned int nrCtrlPnts = 0;
    nrCtrlPnts =  PFPolygon->GetNumberOfControlPoints();
    
    if (nrCtrlPnts <= 2) {
      return;
    }
    
    m_points->SetNumberOfPoints(nrCtrlPnts);
    
    // Create the polygon
    m_polygon->GetPointIds()->SetNumberOfIds(nrCtrlPnts); //make a quad
    
    
    //add controlpoints to vtkPoints and link corresponding point id's
    for (unsigned int i=0; i<nrCtrlPnts; ++i) 
    {
      m_points->InsertPoint(i,(double)PFPolygon->GetWorldControlPoint(i)[0], (double)PFPolygon->GetWorldControlPoint(i)[1], (double)PFPolygon->GetWorldControlPoint(i)[2] );
      m_polygon->GetPointIds()->SetId(i, i);
      
    }
    
    m_polygonsCell->Reset();
    m_polygonsCell->InsertNextCell(m_polygon);   
    
    m_polygonPolyData->SetPoints(m_points);
    m_polygonPolyData->SetPolys(m_polygonsCell);
    
    
    // Visualize
    
    m_VtkPolygonDataMapperGL->SetInput(m_polygonPolyData);
    m_PolygonActor->SetMapper(m_VtkPolygonDataMapperGL);
    m_PolygonAssembly->AddPart(m_PolygonActor);
    
    this->GetDataNode()->SetColor(200.0,200.0,200.0);
    this->GetDataNode()->SetOpacity(0.8);
    
    //guess 1 call might be enough ;)
    m_VtkPolygonDataMapperGL->Modified();
    
  }
  catch(...) { 
//    MITK_INFO << "catch in PlanarPolygonMapper3D GenerateData()"; 
  } 
  
  //MITK_INFO << "_______GENERATE DATA() END_________ \n ===============================";
  
}



/* NEEDS TO BE OPTiMISED!!! */
//template<class TPixelType>
void mitk::PlanarPolygonMapper3D::GenerateDataForRenderer( mitk::BaseRenderer *renderer )
{
  //  bool is3DEnabled;
  //  this->GetDataNode()->GetPropertyValue("PlanarPolygon_3D",is3DEnabled);
  //  if (!is3DEnabled) {
  //    return;
  //  }
  //get the polydata from mapper and then modify points ... thats it what sould happen here
  
  try{
    
    
    mitk::PlanarPolygon* PFPolygon = dynamic_cast< mitk::PlanarPolygon* > (this->GetData());
    
    //get the control points from pf and insert them to vtkPolygon
    unsigned int nrCtrlPnts = 0;
    nrCtrlPnts =  PFPolygon->GetNumberOfControlPoints();
    
    if (nrCtrlPnts <= 2) 
      return;
    
    //if there are 3 points available, draw current polygon in 3D

    if (nrCtrlPnts > 2)
      this->GenerateData();
    
    //update points in polygon
    
    m_points->SetNumberOfPoints(nrCtrlPnts);
    
    // Create the polygon
    m_polygon->GetPointIds()->SetNumberOfIds(nrCtrlPnts); //make a quad
    
    
    //add controlpoints to vtkPoints and link corresponding point id's
    for (unsigned int i=0; i<nrCtrlPnts; ++i) 
    {
      m_points->InsertPoint(i,(double)PFPolygon->GetWorldControlPoint(i)[0], (double)PFPolygon->GetWorldControlPoint(i)[1], (double)PFPolygon->GetWorldControlPoint(i)[2] );
      m_polygon->GetPointIds()->SetId(i, i);
      
    }
    
        
    //updates all polygon pipeline
    m_VtkPolygonDataMapperGL->Modified();
    

    float polyOpaq;
    this->GetDataNode()->GetOpacity(polyOpaq, NULL);
    m_PolygonActor->GetProperty()->SetOpacity((double) polyOpaq);
    
    float temprgb[3];
    this->GetDataNode()->GetColor( temprgb, NULL );
    double trgb[3] = { (double) temprgb[0], (double) temprgb[1], (double) temprgb[2] };
    m_PolygonActor->GetProperty()->SetColor(trgb);
    
    
  }
  catch (...) {
//    MITK_INFO << "catch PolygonMapperr3DgenerateData(baseRenderer)";
  }
}

//template<class TPixelType>
void mitk::PlanarPolygonMapper3D::SetDefaultProperties(mitk::DataNode* node, mitk::BaseRenderer* renderer, bool overwrite)
{
  // node->AddProperty( "PlanarPolygon_3D", mitk::BoolProperty::New( true ), renderer, overwrite  ); 
  Superclass::SetDefaultProperties(node, renderer, overwrite);
  
}

vtkProp* mitk::PlanarPolygonMapper3D::GetVtkProp(mitk::BaseRenderer *renderer)
{
  
  //MITK_INFO << "FiberBundleMapper3D GetVtkProp(renderer)";
  return m_PolygonAssembly;
  
}


void mitk::PlanarPolygonMapper3D::UpdateVtkObjects()
{
  // MITK_INFO << "FiberBundleMapper3D UpdateVtkObjects()";
  
  
}

void mitk::PlanarPolygonMapper3D::SetVtkMapperImmediateModeRendering(vtkMapper *)
{
  
  
  
}



