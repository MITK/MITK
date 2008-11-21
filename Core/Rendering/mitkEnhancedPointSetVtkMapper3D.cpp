/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#include "mitkEnhancedPointSetVtkMapper3D.h"

//#include <sstream>
#include <algorithm>

#include "mitkDataTreeNode.h"
#include "mitkProperties.h"
#include "mitkColorProperty.h"
//#include "mitkVtkPropRenderer.h"

#include <vtkActor.h>

#include <vtkPropAssembly.h>
#include <vtkTubeFilter.h>

#include <vtkSphereSource.h>
#include <vtkCubeSource.h>
#include <vtkConeSource.h>
#include <vtkCylinderSource.h>
#include <vtkProperty.h>
#include <vtkPolyDataMapper.h>
#include <vtkTransformPolyDataFilter.h>

#include <vtkPolyDataAlgorithm.h>


const mitk::PointSet* mitk::EnhancedPointSetVtkMapper3D::GetInput()
{
  return static_cast<const mitk::PointSet * > ( GetData() );
}

mitk::EnhancedPointSetVtkMapper3D::EnhancedPointSetVtkMapper3D()
{
  m_Contour = vtkActor::New();
  m_ContourSource = vtkTubeFilter::New();
  m_PropAssembly = vtkPropAssembly::New();

}

mitk::EnhancedPointSetVtkMapper3D::~EnhancedPointSetVtkMapper3D()
{
  m_Contour->Delete();
  m_ContourSource->Delete();
  m_PropAssembly->Delete();

  // TODO: do cleanup correctly

  // Clean up all remaining actors and poly-data sources
  //std::for_each(m_PointActors.begin(), m_PointActors.end(), &mitk::EnhancedPointSetVtkMapper3D::DeleteVtkObject);

//  std::for_each(m_SphereSources.begin(), m_SphereSources.end(), &mitk::EnhancedPointSetVtgkMapper3D::DeleteVtkObject);
//  std::for_each(m_CubeSources.begin(), m_CubeSources.end(), &mitk::EnhancedPointSetVtkMapper3D::DeleteVtkObject);
//  std::for_each(m_ConeSources.begin(), m_ConeSources.end(), &mitk::EnhancedPointSetVtkMapper3D::DeleteVtkObject);
//  std::for_each(m_CylinderSources.begin(), m_CylinderSources.end(), &mitk::EnhancedPointSetVtkMapper3D::DeleteVtkObject);
//
}

void mitk::EnhancedPointSetVtkMapper3D::ReleaseGraphicsResources(vtkWindow * /*renWin*/)
{
  // TODO: Do we have to call this for all actors??
  //m_Actor->ReleaseGraphicsResources(renWin);
}

void mitk::EnhancedPointSetVtkMapper3D::UpdateVtkObjects()
{
  // get and update the PointSet
  const mitk::PointSet* pointset = this->GetInput();
  //pointset->Update();
  int timestep = this->GetTimestep();
 
  mitk::PointSet::DataType* itkPointSet = pointset->GetPointSet( timestep );
  mitk::PointSet::PointsContainer* points = itkPointSet->GetPoints();
  mitk::PointSet::PointDataContainer* pointData = itkPointSet->GetPointData();
  
  assert(points->Size() == pointData->Size());

  mitk::PointSet::PointsIterator pIt;
  mitk::PointSet::PointDataIterator pdIt;
  
  /* search removed points and delete the corresponding source/actor/mapper objects */
  for (ActorMap::iterator it = m_PointActors.begin(); it != m_PointActors.end(); )
  {
    PointIdentifier id = it->first;
    if (points->IndexExists(id))
    {
      this->RemoveEntryFromSourceMaps(id);
      if (it->second.first != NULL)
        it->second.first->Delete(); // Delete actor, which deletes mapper too (reference count)
      ActorMap::iterator er = it;
      ++it;
      m_PointActors.erase(er); // erase element from map, get pointer to following element
    }
    else
      ++it;
  }

  /* iterate over each point in the pointset and create/update polydata and its properties */
  for (pIt = points->Begin(), pdIt = pointData->Begin(); pIt != itkPointSet->GetPoints()->End(); ++pIt, ++pdIt)
  {
    PointIdentifier pointID = pIt->Index();
    assert (pointID == pdIt->Index());

    mitk::PointSet::PointType point = pIt->Value();
    mitk::PointSet::PointDataType data = pdIt->Value();
    ActorMap::iterator aIt = m_PointActors.find(pointID);

    /* Create/Update sources for the point */
    vtkActor* a = NULL;
    bool newPoint = (aIt == m_PointActors.end()); // current point is new
    bool specChanged = (aIt != m_PointActors.end());  // point spec of current point has changed

    if (newPoint)  // point non-existing
    { // create actor and mapper for the new point
      a = vtkActor::New();
      vtkPolyDataMapper* m = vtkPolyDataMapper::New();
      a->SetMapper(m);
      m->UnRegister( NULL );
      m_PointActors[pointID] = std::make_pair(a, data.pointSpec);
    }
    else
    {
      a = aIt->second.first;
      if (specChanged)  // point exists, but point spec has changed
      {     
        this->RemoveEntryFromSourceMaps( pointID );
      }
    }
    if (  newPoint   // new point 
       || (!newPoint && specChanged) ) // OR existing point but point spec changed
    {
      vtkPolyDataAlgorithm* source = NULL;  // works only in VTK 5+
      switch (data.pointSpec)  // add to new map
      {                                               //TODO: look up representation in a representationlookuptable
      case PTSTART:        //cube
        m_CubeSources[pointID] = vtkCubeSource::New();
        source = m_CubeSources[pointID];
        break;
      case PTCORNER:          //cone
        m_ConeSources[pointID] = vtkConeSource::New();
        source = m_ConeSources[pointID];
        break;
      case PTEDGE:          // cylinder
        m_CylinderSources[pointID] = vtkCylinderSource::New();
        source = m_CylinderSources[pointID];
        break;
      case PTUNDEFINED:     // sphere
      case PTEND:
      default:
        m_SphereSources[pointID] = vtkSphereSource::New();
        source = m_SphereSources[pointID];
        break;
      }
      vtkPolyDataMapper* m = dynamic_cast<vtkPolyDataMapper*>(a->GetMapper());
      assert(m != NULL);
      m->SetInput(source->GetOutput());
      aIt->second.second = data.pointSpec; // update point spec in actormap
    }

    /* update properties */
      // visibility
    bool vis = true;
    this->GetDataTreeNode()->GetBoolProperty("visibility", vis /*, renderer*/);  //TODO look for visibility lookup table for pointwise visibility, TODO2: get the property from the correct renderer!
    // TODO apply visibility to vtkProperty of the actor a.
    // TODO continue with all other properties
  } // for each point 
}



void mitk::EnhancedPointSetVtkMapper3D::GenerateData()
{
  ////create new vtk render objects (e.g. sphere for a point)
  //this->CreateVTKRenderObjects();

  //apply props
  //Superclass::ApplyProperties( m_ContourActor, NULL );
  //this->ApplyProperties(NULL);

}


void mitk::EnhancedPointSetVtkMapper3D::GenerateData( mitk::BaseRenderer * /*renderer*/ )
{

//// just for testing, always call CreateVTKRenderObjects()
//  mitk::FloatProperty::Pointer pointSizeProp = dynamic_cast<mitk::FloatProperty *>(this->GetDataTreeNode()->GetProperty("pointsize"));
//  mitk::FloatProperty::Pointer contourSizeProp = dynamic_cast<mitk::FloatProperty *>(this->GetDataTreeNode()->GetProperty("contoursize"));
//  // only create new vtk render objects if property values were changed
//  if ( pointSizeProp.IsNotNull() &&  contourSizeProp.IsNotNull() )
//  {
//    if (m_PointSize!=pointSizeProp->GetValue() || m_ContourRadius!= contourSizeProp->GetValue())
//    {
//      this->CreateVTKRenderObjects();
//    }
//  }
//  //this->CreateVTKRenderObjects();  //just for testing
//
//  Superclass::ApplyProperties( m_ContourActor, renderer );
//  this->ApplyProperties(renderer);
//
//  if(IsVisible(renderer)==false)
//  {
//    m_UnselectedActor->VisibilityOff();
//    m_SelectedActor->VisibilityOff();
//    m_ContourActor->VisibilityOff();
//    return;
//  }
//
//  bool showPoints = true;
//  this->GetDataTreeNode()->GetBoolProperty("show points", showPoints);
//  if(showPoints)
//  {
//    m_UnselectedActor->VisibilityOn();
//    m_SelectedActor->VisibilityOn();
//  }
//  else
//  {
//    m_UnselectedActor->VisibilityOff();
//    m_SelectedActor->VisibilityOff();
//  }
//
//  if(dynamic_cast<mitk::FloatProperty *>(this->GetDataTreeNode()->GetProperty("opacity")) != NULL)
//  {  
//    mitk::FloatProperty::Pointer pointOpacity =dynamic_cast<mitk::FloatProperty *>(this->GetDataTreeNode()->GetProperty("opacity"));
//    float opacity = pointOpacity->GetValue();
//    m_ContourActor->GetProperty()->SetOpacity(opacity);
//    m_UnselectedActor->GetProperty()->SetOpacity(opacity);
//    m_SelectedActor->GetProperty()->SetOpacity(opacity);
//  }
//
//  bool makeContour = false;
//  this->GetDataTreeNode()->GetBoolProperty("show contour", makeContour);
//  if (makeContour)
//  {
//    m_ContourActor->VisibilityOn();
//  }
//  else
//  {
//    m_ContourActor->VisibilityOff();
//  }
//
//
//  // Get the TimeSlicedGeometry of the input object
//  mitk::PointSet::Pointer input  = const_cast<mitk::PointSet*>(this->GetInput());
//  const TimeSlicedGeometry* inputTimeGeometry = input->GetTimeSlicedGeometry();
//  if (( inputTimeGeometry == NULL ) || ( inputTimeGeometry->GetTimeSteps() == 0 ))
//  {
//    m_PointsAssembly->VisibilityOff();
//    return;
//  }
//
//  if ( inputTimeGeometry->IsValidTime( this->GetTimestep() ) == false )
//  {
//    m_PointsAssembly->VisibilityOff();
//    return;
//  }
}


void mitk::EnhancedPointSetVtkMapper3D::UpdateVtkTransform()
{
  //vtkLinearTransform * vtktransform = 
  //  this->GetDataTreeNode()->GetVtkTransform(this->GetTimestep());

  //m_SelectedActor->SetUserTransform(vtktransform);
  //m_UnselectedActor->SetUserTransform(vtktransform);
  //m_ContourActor->SetUserTransform(vtktransform);
}

void mitk::EnhancedPointSetVtkMapper3D::ApplyProperties(mitk::BaseRenderer* /*renderer*/)
{
  ////check for color props and use it for rendering of selected/unselected points and contour 
  ////due to different params in VTK (double/float) we have to convert!

  ////vars to convert to
  //vtkFloatingPointType unselectedColor[4]={1.0f,1.0f,0.0f,1.0f};//yellow
  //vtkFloatingPointType selectedColor[4]={1.0f,0.0f,0.0f,1.0f};//red
  //vtkFloatingPointType contourColor[4]={1.0f,0.0f,0.0f,1.0f};//red

  ////different types for color!!!
  //mitk::Color tmpColor;
  //double opacity = 1.0;

  ////check if there is an unselected property
  //if (dynamic_cast<mitk::ColorProperty*>(this->GetDataTreeNode()->GetPropertyList(renderer)->GetProperty("unselectedcolor")) != NULL)
  //{
  //  tmpColor = dynamic_cast<mitk::ColorProperty *>(this->GetDataTreeNode()->GetPropertyList(renderer)->GetProperty("unselectedcolor"))->GetValue();
  //  unselectedColor[0] = tmpColor[0];
  //  unselectedColor[1] = tmpColor[1];
  //  unselectedColor[2] = tmpColor[2];
  //  unselectedColor[3] = 1.0f; //!!define a new ColorProp to be able to pass alpha value
  //}
  //else if (dynamic_cast<mitk::ColorProperty*>(this->GetDataTreeNode()->GetPropertyList(NULL)->GetProperty("unselectedcolor")) != NULL)
  //{
  //  tmpColor = dynamic_cast<mitk::ColorProperty *>(this->GetDataTreeNode()->GetPropertyList(NULL)->GetProperty("unselectedcolor"))->GetValue();
  //  unselectedColor[0] = tmpColor[0];
  //  unselectedColor[1] = tmpColor[1];
  //  unselectedColor[2] = tmpColor[2];
  //  unselectedColor[3] = 1.0f; //!!define a new ColorProp to be able to pass alpha value
  //}
  //else
  //{
  //  //check if the node has a color
  //  float unselectedColorTMP[4]={1.0f,1.0f,0.0f,1.0f};//yellow
  //  m_DataTreeNode->GetColor(unselectedColorTMP, NULL);
  //  unselectedColor[0] = unselectedColorTMP[0];
  //  unselectedColor[1] = unselectedColorTMP[1];
  //  unselectedColor[2] = unselectedColorTMP[2];
  //  //unselectedColor[3] stays 1.0f
  //}

  ////get selected property
  //if (dynamic_cast<mitk::ColorProperty*>(this->GetDataTreeNode()->GetPropertyList(renderer)->GetProperty("selectedcolor")) != NULL)
  //{
  //  tmpColor = dynamic_cast<mitk::ColorProperty *>(this->GetDataTreeNode()->GetPropertyList(renderer)->GetProperty("selectedcolor"))->GetValue();
  //  selectedColor[0] = tmpColor[0];
  //  selectedColor[1] = tmpColor[1];
  //  selectedColor[2] = tmpColor[2];
  //  selectedColor[3] = 1.0f;
  //}
  //else if (dynamic_cast<mitk::ColorProperty*>(this->GetDataTreeNode()->GetPropertyList(NULL)->GetProperty("selectedcolor")) != NULL)
  //{
  //  tmpColor = dynamic_cast<mitk::ColorProperty *>(this->GetDataTreeNode()->GetPropertyList(NULL)->GetProperty("selectedcolor"))->GetValue();
  //  selectedColor[0] = tmpColor[0];
  //  selectedColor[1] = tmpColor[1];
  //  selectedColor[2] = tmpColor[2];
  //  selectedColor[3] = 1.0f;
  //}

  ////get contour property
  //if (dynamic_cast<mitk::ColorProperty*>(this->GetDataTreeNode()->GetPropertyList(renderer)->GetProperty("contourcolor")) != NULL)
  //{
  //  tmpColor = dynamic_cast<mitk::ColorProperty *>(this->GetDataTreeNode()->GetPropertyList(renderer)->GetProperty("contourcolor"))->GetValue();
  //  contourColor[0] = tmpColor[0];
  //  contourColor[1] = tmpColor[1];
  //  contourColor[2] = tmpColor[2];
  //  contourColor[3] = 1.0f;
  //}
  //else if (dynamic_cast<mitk::ColorProperty*>(this->GetDataTreeNode()->GetPropertyList(NULL)->GetProperty("contourcolor")) != NULL)
  //{
  //  tmpColor = dynamic_cast<mitk::ColorProperty *>(this->GetDataTreeNode()->GetPropertyList(NULL)->GetProperty("contourcolor"))->GetValue();
  //  contourColor[0] = tmpColor[0];
  //  contourColor[1] = tmpColor[1];
  //  contourColor[2] = tmpColor[2];
  //  contourColor[3] = 1.0f;
  //}

  //if(dynamic_cast<mitk::FloatProperty *>(this->GetDataTreeNode()->GetPropertyList(renderer)->GetProperty("opacity")) != NULL)
  //{
  //  mitk::FloatProperty::Pointer pointOpacity =dynamic_cast<mitk::FloatProperty *>(this->GetDataTreeNode()->GetPropertyList(renderer)->GetProperty("opacity"));
  //  opacity = pointOpacity->GetValue();
  //}
  //else if(dynamic_cast<mitk::FloatProperty *>(this->GetDataTreeNode()->GetPropertyList(NULL)->GetProperty("opacity")) != NULL)
  //{
  //  mitk::FloatProperty::Pointer pointOpacity =dynamic_cast<mitk::FloatProperty *>(this->GetDataTreeNode()->GetPropertyList(NULL)->GetProperty("opacity"));
  //  opacity = pointOpacity->GetValue();
  //}
  ////finished color / opacity fishing!

  ////check if a contour shall be drawn
  //bool makeContour = false;
  //this->GetDataTreeNode()->GetBoolProperty("show contour", makeContour, renderer);
  //int visibleBefore = m_ContourActor->GetVisibility();
  //if(makeContour && (m_ContourActor != NULL) )
  //{
  //  if ( visibleBefore == 0)//was not visible before, so create it.
  //    this->CreateContour(renderer);
  //  m_ContourActor->GetProperty()->SetColor(contourColor);
  //  m_ContourActor->GetProperty()->SetOpacity(opacity);
  //}

  //m_SelectedActor->GetProperty()->SetColor(selectedColor);
  //m_SelectedActor->GetProperty()->SetOpacity(opacity);

  //m_UnselectedActor->GetProperty()->SetColor(unselectedColor);
  //m_UnselectedActor->GetProperty()->SetOpacity(opacity);

}

void mitk::EnhancedPointSetVtkMapper3D::CreateContour(mitk::BaseRenderer* /*renderer*/)
{
  //vtkAppendPolyData* vtkContourPolyData = vtkAppendPolyData::New();
  //vtkPolyDataMapper* vtkContourPolyDataMapper = vtkPolyDataMapper::New(); 

  //vtkPoints *points = vtkPoints::New();
  //vtkCellArray *polys = vtkCellArray::New();

  //mitk::PointSet::PointsContainer::Iterator pointsIter;
  //mitk::PointSet::PointDataContainer::Iterator pointDataIter;
  //int j;

  //// get and update the PointSet
  //mitk::PointSet::Pointer input  = const_cast<mitk::PointSet*>(this->GetInput());
  //input->Update();

  //int timestep = this->GetTimestep();
  //mitk::PointSet::DataType::Pointer itkPointSet = input->GetPointSet( timestep );
  //if ( itkPointSet.GetPointer() == NULL) 
  //{
  //  return;
  //}

  //for (j=0, pointsIter=itkPointSet->GetPoints()->Begin(); pointsIter!=itkPointSet->GetPoints()->End() ; pointsIter++,j++)
  //{
  //  vtkIdType cell[2] = {j-1,j};
  //  mitk::Point3D point1;
  //  point1 = input->GetPoint(pointsIter->Index(), timestep);
  //  points->InsertPoint(j,point1[0],point1[1],point1[2]);
  //  if (j>0)
  //    polys->InsertNextCell(2,cell);
  //}

  //bool close = false;
  //this->GetDataTreeNode()->GetBoolProperty( "close contour", close, renderer );
  //if (close) 
  //{
  //  vtkIdType cell[2] = {j-1,0};
  //  polys->InsertNextCell(2,cell);
  //}

  //vtkPolyData* contour = vtkPolyData::New();
  //contour->SetPoints(points);
  //points->Delete();
  //contour->SetLines(polys);
  //polys->Delete();
  //contour->Update();

  //vtkTubeFilter* tubeFilter = vtkTubeFilter::New();
  //tubeFilter->SetNumberOfSides( 12 );
  //tubeFilter->SetInput(contour);
  //contour->Delete();

  ////check for property contoursize. 
  //m_ContourRadius = 0.5;
  //mitk::FloatProperty::Pointer contourSizeProp = dynamic_cast<mitk::FloatProperty *>(this->GetDataTreeNode()->GetProperty("contoursize") );

  //if (contourSizeProp.IsNotNull())
  //  m_ContourRadius = contourSizeProp->GetValue();

  //tubeFilter->SetRadius( m_ContourRadius );
  //tubeFilter->Update();

  ////add to pipeline
  //vtkContourPolyData->AddInput(tubeFilter->GetOutput());
  //tubeFilter->Delete();
  //vtkContourPolyDataMapper->SetInput(vtkContourPolyData->GetOutput());
  //vtkContourPolyData->Delete();

  ////create a new instance of the actor
  //m_ContourActor->Delete();
  //m_ContourActor = vtkActor::New();

  //m_ContourActor->SetMapper(vtkContourPolyDataMapper);
  //vtkContourPolyDataMapper->Delete();

  //m_PointsAssembly->AddPart(m_ContourActor);
}


void mitk::EnhancedPointSetVtkMapper3D::SetDefaultProperties(mitk::DataTreeNode* node, mitk::BaseRenderer* renderer, bool overwrite)
{
  node->AddProperty( "line width", mitk::IntProperty::New(2), renderer, overwrite );
  node->AddProperty( "pointsize", mitk::FloatProperty::New(1.0), renderer, overwrite);
  node->AddProperty( "selectedcolor", mitk::ColorProperty::New(1.0f, 0.0f, 0.0f), renderer, overwrite);  //red
  node->AddProperty( "color", mitk::ColorProperty::New(1.0f, 1.0f, 0.0f), renderer, overwrite);  //yellow
  node->AddProperty( "show contour", mitk::BoolProperty::New(false), renderer, overwrite );
  node->AddProperty( "contourcolor", mitk::ColorProperty::New(1.0f, 0.0f, 0.0f), renderer, overwrite);
  node->AddProperty( "contoursize", mitk::FloatProperty::New(0.5), renderer, overwrite );
  node->AddProperty( "close contour", mitk::BoolProperty::New(false), renderer, overwrite );
  node->AddProperty( "show points", mitk::BoolProperty::New(true), renderer, overwrite );
  node->AddProperty( "show label", mitk::BoolProperty::New(false), renderer, overwrite );
  node->AddProperty( "label", mitk::StringProperty::New("P"), renderer, overwrite );
  node->AddProperty( "opacity", mitk::FloatProperty::New(1.0), renderer, overwrite );
  Superclass::SetDefaultProperties(node, renderer, overwrite);
}


void mitk::EnhancedPointSetVtkMapper3D::DeleteVtkObject( vtkObject* o)
{
  if (o != NULL)
    o->Delete();
}


void mitk::EnhancedPointSetVtkMapper3D::RemoveEntryFromSourceMaps( mitk::PointSet::PointIdentifier pointID )
{
  ActorMap::iterator aIt = m_PointActors.find(pointID);
  if (aIt == m_PointActors.end())
    return;

  switch (aIt->second.second)  // erase in old map
  {      //TODO: look up representation in a representationlookuptable
  case PTSTART:        //cube
    m_CubeSources[pointID]->Delete();
    m_CubeSources.erase(pointID);
    break;

  case PTCORNER:          //cone
    m_ConeSources[pointID]->Delete();
    m_ConeSources.erase(pointID);
    break;

  case PTEDGE:          // cylinder
    m_CylinderSources[pointID]->Delete();
    m_CylinderSources.erase(pointID);
    break;

  case PTUNDEFINED:     // sphere
  case PTEND:
  default:
    m_SphereSources[pointID]->Delete();
    m_SphereSources.erase(pointID);          
    break;
  }        
}
