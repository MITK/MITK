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
#include "mitkLookupTables.h"

#include "mitkColorProperty.h"
//#include "mitkVtkPropRenderer.h"

#include <vtkActor.h>

#include <vtkAssembly.h>
#include <vtkProp3DCollection.h>
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
  m_PropAssembly = vtkAssembly::New();
}

vtkProp* mitk::EnhancedPointSetVtkMapper3D::GetVtkProp(mitk::BaseRenderer*  /*renderer*/)
{
  return m_PropAssembly;
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
    if (!points->IndexExists(id))
    {
      this->RemoveEntryFromSourceMaps(id);
      m_PropAssembly->GetParts()->RemoveItem(it->second.first); // remove from prop assembly
      if (it->second.first != NULL)
        it->second.first->Delete(); // Delete actor, which deletes mapper too (reference count)
      ActorMap::iterator er = it; // save iterator for deleting
      ++it;  // advance iterator to next object
      m_PointActors.erase(er); // erase element from map. This invalidates er, therefore we had to advance it before deletion.
    }
    else
      ++it;
  }
  
  /* iterate over each point in the pointset and create corresponding vtk objects */
  for (pIt = points->Begin(), pdIt = pointData->Begin(); pIt != itkPointSet->GetPoints()->End(); ++pIt, ++pdIt)
  {
    PointIdentifier pointID = pIt->Index();
    assert (pointID == pdIt->Index());

    mitk::PointSet::PointType point = pIt->Value();
    mitk::PointSet::PointDataType data = pdIt->Value();
    
    ActorMap::iterator aIt = m_PointActors.find(pointID); // Does an actor exist for the point?

    /* Create/Update sources for the point */
    vtkActor* a = NULL;
    bool newPoint = (aIt == m_PointActors.end()); // current point is new
    bool specChanged = (!newPoint && data.pointSpec != aIt->second.second); // point spec of current point has changed

    if (newPoint)  // point did not exist before, we have to create vtk objects for it
    { // create actor and mapper for the new point
      a = vtkActor::New();
      vtkPolyDataMapper* m = vtkPolyDataMapper::New();
      a->SetMapper(m);
      m->UnRegister( NULL );
      aIt = m_PointActors.insert(std::make_pair(pointID, std::make_pair(a, data.pointSpec))).first;  // insert element and update actormap iterator to point to new element
      m_PropAssembly->AddPart(a);
    }
    else
    {
      a = aIt->second.first;
      if (specChanged)  // point exists, but point spec has changed
      {     
        this->RemoveEntryFromSourceMaps( pointID );
      }
    }
    if ( newPoint || specChanged ) // new point OR existing point but point spec changed
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
  } // for each point  
}



void mitk::EnhancedPointSetVtkMapper3D::GenerateData()
{
  this->UpdateVtkObjects();
}


void mitk::EnhancedPointSetVtkMapper3D::ApplyProperties( mitk::BaseRenderer * renderer )
{
  /* iterate over all points in pointset and apply properties to corresponding vtk objects */
  // get and update the PointSet
  const mitk::PointSet* pointset = this->GetInput();
  int timestep = this->GetTimestep();
  mitk::PointSet::DataType* itkPointSet = pointset->GetPointSet( timestep );
  mitk::PointSet::PointsContainer* points = itkPointSet->GetPoints();
  mitk::PointSet::PointDataContainer* pointData = itkPointSet->GetPointData();
  assert(points->Size() == pointData->Size());
  mitk::PointSet::PointsIterator pIt;
  mitk::PointSet::PointDataIterator pdIt;
  mitk::DataTreeNode* n = this->GetDataTreeNode();
  assert(n != NULL);

  for (pIt = points->Begin(), pdIt = pointData->Begin(); pIt != itkPointSet->GetPoints()->End(); ++pIt, ++pdIt)  // for each point in the pointset
  {
    PointIdentifier pointID = pIt->Index();
    assert (pointID == pdIt->Index());

    mitk::PointSet::PointType point = pIt->Value();
    mitk::PointSet::PointDataType data = pdIt->Value();

    ActorMap::iterator aIt = m_PointActors.find(pointID); // Does an actor exist for the point?
    assert(aIt != m_PointActors.end()); // UpdateVtkObjects() must ensure that actor exists
    
    vtkActor* a = aIt->second.first;
    assert(a != NULL);

    SetVtkMapperImmediateModeRendering(a->GetMapper());

    /* update properties */
    // visibility
    bool pointVisibility = true;
    bool visValueFound = false;
    mitk::BaseProperty* visProp = n->GetProperty("visibility", renderer);
    mitk::BoolLookupTableProperty* visLTProp = dynamic_cast<mitk::BoolLookupTableProperty*>(visProp);
    if (visLTProp != NULL)
    {
      mitk::BoolLookupTable visLookupTable = visLTProp->GetValue();
      //if (visLookupTable != NULL)
      //{
        try
        {
          pointVisibility = visLookupTable.GetTableValue(pointID);
          visValueFound = true;
        }
        catch (...)
        {
        }
      //}
    }
    if (visValueFound == false)
    {
      pointVisibility = n->IsVisible(renderer, "show points");  // use BoolProperty instead
    }
    a->SetVisibility(pointVisibility);

    // opacity
    float opacity = 1.0;
    bool opValueFound = false;
    mitk::BaseProperty* opProp = n->GetProperty("opacity", renderer);
    mitk::FloatLookupTableProperty* opLTProp = dynamic_cast<mitk::FloatLookupTableProperty*>(opProp);
    if (opLTProp != NULL)
    {
      mitk::FloatLookupTable opLookupTable = opLTProp->GetValue();
      //if (opLookupTable != NULL)
      //{
        try
        {
          opacity = opLookupTable.GetTableValue(pointID);
          opValueFound = true;
        }
        catch (...)
        {
        }
      //}
    }
    if (opValueFound == false)
    {
      n->GetOpacity(opacity, renderer);
    }
    a->GetProperty()->SetOpacity(opacity);
////////////////////// continue here ///////////////////

    // pointsize & point position
    float pointSize = 1.0;
    n->GetFloatProperty( "pointsize", pointSize, renderer);
    switch (data.pointSpec)
    {                                               //TODO: look up representation in a representationlookuptable
    case PTSTART:        //cube
      m_CubeSources[pointID]->SetXLength(pointSize);
      m_CubeSources[pointID]->SetYLength(pointSize);
      m_CubeSources[pointID]->SetZLength(pointSize);
      //m_CubeSources[pointID]->SetCenter(pos[0], pos[1], pos[2]);
      break;
    case PTCORNER:          //cone
      m_ConeSources[pointID]->SetRadius(pointSize/2);
      m_ConeSources[pointID]->SetHeight(pointSize);
      m_ConeSources[pointID]->SetResolution(2); // two crossed triangles. Maybe introduce an extra property for 
      //m_ConeSources[pointID]->SetCenter(pos[0], pos[1], pos[2]);
      break;
    case PTEDGE:          // cylinder
      m_CylinderSources[pointID]->SetRadius(pointSize/2);
      m_CylinderSources[pointID]->SetHeight(pointSize);
      m_CylinderSources[pointID]->CappingOn();
      m_CylinderSources[pointID]->SetResolution(6);
      //m_CylinderSources[pointID]->SetCenter(pos[0], pos[1], pos[2]);
      break;
    case PTUNDEFINED:     // sphere
    case PTEND:
    default:
      m_SphereSources[pointID]->SetRadius(pointSize/2);
      m_SphereSources[pointID]->SetThetaResolution(10);
      m_SphereSources[pointID]->SetPhiResolution(10);
      //m_SphereSources[pointID]->SetCenter(pos[0], pos[1], pos[2]);
      break;
    }

    // set position
    mitk::Point3D pos = pIt->Value();
    aIt->second.first->SetPosition(pos[0], pos[1], pos[2]);

    // selectedcolor & color
    float color[3];
    if (data.selected)
      n->GetColor(color, renderer, "selectedcolor");
    else
      n->GetColor(color, renderer, "unselectedcolor");  // TODO: What about "color" property? 2D Mapper only uses unselected and selected color properties
    a->GetProperty()->SetColor(color[0], color[1], color[2]);

    // TODO: label property
  }
  //TODO test different pointSpec
  // TODO "line width" "show contour" "contourcolor" "contoursize" "close contour" "show label", "label"
  // TODO "show points" vs "visibility"  - is visibility evaluated at all? in a superclass maybe? 
  // TODO create lookup tables for all properties that should be evaluated per point. also create editor widgets for these lookup tables!
  // TODO check if property changes and pointset changes are reflected in the render window immediately.
  // TODO check behavior with large PointSets
  // TODO check for memory leaks on adding/deleting points
}


void mitk::EnhancedPointSetVtkMapper3D::GenerateData( mitk::BaseRenderer * renderer )
{
  ApplyProperties(renderer);
}


void mitk::EnhancedPointSetVtkMapper3D::UpdateVtkTransform(mitk::BaseRenderer * /*renderer*/)
{
  // TODO: apply new transform if time step changed

  //vtkLinearTransform * vtktransform = 
  //  this->GetDataTreeNode()->GetVtkTransform(this->GetTimestep());

  //m_SelectedActor->SetUserTransform(vtktransform);
  //m_UnselectedActor->SetUserTransform(vtktransform);
  //m_ContourActor->SetUserTransform(vtktransform);
}


void mitk::EnhancedPointSetVtkMapper3D::SetDefaultProperties(mitk::DataTreeNode* node, mitk::BaseRenderer* renderer, bool overwrite)
{
  node->AddProperty( "line width", mitk::IntProperty::New(2), renderer, overwrite );
  node->AddProperty( "pointsize", mitk::FloatProperty::New(1.0), renderer, overwrite);
  node->AddProperty( "selectedcolor", mitk::ColorProperty::New(1.0f, 1.0f, 0.0f), renderer, overwrite);  //yellow for selected
  node->AddProperty( "unselectedcolor", mitk::ColorProperty::New(0.5f, 1.0f, 0.5f), renderer, overwrite);  // middle green for unselected
  node->AddProperty( "color", mitk::ColorProperty::New(1.0f, 0.0f, 0.0f), renderer, overwrite);  // red as standard
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
