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

#include "mitkLiveWireTool2D.h"

#include "mitkToolManager.h"
#include "mitkBaseRenderer.h"
#include "mitkRenderingManager.h"

#include "mitkLiveWireTool2D.xpm"

#include <mitkInteractionConst.h>
#include <mitkGlobalInteraction.h>

#include "mitkContourUtils.h"
#include "mitkContour.h"

#include <itkGradientMagnitudeImageFilter.h>

// us
#include <usModule.h>
#include <usModuleResource.h>
#include <usGetModuleContext.h>
#include <usModuleContext.h>

namespace mitk {
  MITK_TOOL_MACRO(Segmentation_EXPORT, LiveWireTool2D, "LiveWire tool");
}


mitk::LiveWireTool2D::LiveWireTool2D()
:SegTool2D("LiveWireTool")
{

  // great magic numbers
  CONNECT_ACTION( AcINITNEWOBJECT, OnInitLiveWire );
  CONNECT_ACTION( AcADDPOINT, OnAddPoint );
  CONNECT_ACTION( AcMOVE, OnMouseMoveNoDynamicCosts );
  CONNECT_ACTION( AcCHECKPOINT, OnCheckPoint );
  CONNECT_ACTION( AcFINISH, OnFinish );
  CONNECT_ACTION( AcDELETEPOINT, OnLastSegmentDelete );
  CONNECT_ACTION( AcADDLINE, OnMouseMoved );
}



mitk::LiveWireTool2D::~LiveWireTool2D()
{
  this->m_WorkingContours.clear();
  this->m_EditingContours.clear();
}



float mitk::LiveWireTool2D::CanHandleEvent( StateEvent const *stateEvent) const
{
  mitk::PositionEvent const  *positionEvent =
  dynamic_cast <const mitk::PositionEvent *> (stateEvent->GetEvent());

  //Key event handling:
  if (positionEvent == NULL)
  {
    //check for delete and escape event
    if(stateEvent->GetId() == 12 || stateEvent->GetId() == 14)
    {
      return 1.0;
    }
    //check, if the current state has a transition waiting for that key event.
    else if (this->GetCurrentState()->GetTransition(stateEvent->GetId())!=NULL)
    {
      return 0.5;
    }
    else
    {
      return 0.0;
    }
  }
  else
  {
    if ( positionEvent->GetSender()->GetMapperID() != BaseRenderer::Standard2D )
      return 0.0; // we don't want anything but 2D

    return 1.0;
  }

}

const char** mitk::LiveWireTool2D::GetXPM() const
{
  return mitkLiveWireTool2D_xpm;
}

us::ModuleResource mitk::LiveWireTool2D::GetIconResource() const
{
  us::Module* module = us::GetModuleContext()->GetModule();
  us::ModuleResource resource = module->GetResource("LiveWire_48x48.png");
  return resource;
}

us::ModuleResource mitk::LiveWireTool2D::GetCursorIconResource() const
{
  us::Module* module = us::GetModuleContext()->GetModule();
  us::ModuleResource resource = module->GetResource("LiveWire_Cursor_32x32.png");
  return resource;
}

const char* mitk::LiveWireTool2D::GetName() const
{
  return "Live Wire";
}

void mitk::LiveWireTool2D::Activated()
{
  Superclass::Activated();
}

void mitk::LiveWireTool2D::Deactivated()
{
  this->ClearContours();

  Superclass::Deactivated();
}

void mitk::LiveWireTool2D::ClearContours()
{
  // for all contours in list (currently created by tool)
  std::vector< std::pair<mitk::DataNode::Pointer, mitk::PlaneGeometry::Pointer> >::iterator iter = this->m_WorkingContours.begin();
  while(iter != this->m_WorkingContours.end() )
  {
    //remove contour node from datastorage
    m_ToolManager->GetDataStorage()->Remove( iter->first );

    ++iter;
  }

  this->m_WorkingContours.clear();

  // for all contours in list (currently created by tool)
  std::vector< std::pair<mitk::DataNode::Pointer, mitk::PlaneGeometry::Pointer> >::iterator itEditingContours = this->m_EditingContours.begin();
  while(itEditingContours != this->m_EditingContours.end() )
  {
      //remove contour node from datastorage
      m_ToolManager->GetDataStorage()->Remove( itEditingContours->first );
      ++itEditingContours;
  }

  this->m_EditingContours.clear();

  std::vector< mitk::ContourModelLiveWireInteractor::Pointer >::iterator itLiveWireInteractors = this->m_LiveWireInteractors.begin();
  while(itLiveWireInteractors != this->m_LiveWireInteractors.end() )
  {
      // remove interactors from globalInteraction instance
      mitk::GlobalInteraction::GetInstance()->RemoveInteractor( *itLiveWireInteractors );
      ++itLiveWireInteractors;
  }

  this->m_LiveWireInteractors.clear();
}

void mitk::LiveWireTool2D::ConfirmSegmentation()
{
  DataNode* workingNode( m_ToolManager->GetWorkingData(0) );
  assert ( workingNode );

  Image* workingImage = dynamic_cast<Image*>(workingNode->GetData());
  assert ( workingImage );

  ContourUtils::Pointer contourUtils = mitk::ContourUtils::New();

  // for all contours in list (currently created by tool)
  std::vector< std::pair<mitk::DataNode::Pointer, mitk::PlaneGeometry::Pointer> >::iterator itWorkingContours = this->m_WorkingContours.begin();
  while(itWorkingContours != this->m_WorkingContours.end() )
  {
    // if node contains data
    if( itWorkingContours->first->GetData() )
    {

      // if this is a contourModel
      mitk::ContourModel* contourModel = dynamic_cast<mitk::ContourModel*>(itWorkingContours->first->GetData());
      if( contourModel )
      {

        // for each timestep of this contourModel
        for( int currentTimestep = 0; currentTimestep < contourModel->GetTimeSlicedGeometry()->GetTimeSteps(); currentTimestep++)
        {

          //get the segmentation image slice at current timestep
          mitk::Image::Pointer workingSlice = this->GetAffectedImageSliceAs2DImage(itWorkingContours->second, workingImage, currentTimestep);

          mitk::ContourModel::Pointer projectedContour = contourUtils->ProjectContourTo2DSlice(workingSlice, contourModel, true, false);
          contourUtils->FillContourInSlice(projectedContour, workingSlice, 1.0);

          //write back to image volume
          this->WriteBackSegmentationResult(itWorkingContours->second, workingSlice, currentTimestep);
        }

        //remove contour node from datastorage
    //    m_ToolManager->GetDataStorage()->Remove( itWorkingContours->first );
      }
    }

    ++itWorkingContours;
  }
/*
  this->m_WorkingContours.clear();

  // for all contours in list (currently created by tool)
  std::vector< std::pair<mitk::DataNode::Pointer, mitk::PlaneGeometry::Pointer> >::iterator itEditingContours = this->m_EditingContours.begin();
  while(itEditingContours != this->m_EditingContours.end() )
  {
      //remove contour node from datastorage
      m_ToolManager->GetDataStorage()->Remove( itEditingContours->first );
      ++itEditingContours;
  }

  this->m_EditingContours.clear();

  std::vector< mitk::ContourModelLiveWireInteractor::Pointer >::iterator itLiveWireInteractors = this->m_LiveWireInteractors.begin();
  while(itLiveWireInteractors != this->m_LiveWireInteractors.end() )
  {
      // remove interactors from globalInteraction instance
      mitk::GlobalInteraction::GetInstance()->RemoveInteractor( *itLiveWireInteractors );
      ++itLiveWireInteractors;
  }

  this->m_LiveWireInteractors.clear();
*/
  this->ClearContours();
}

bool mitk::LiveWireTool2D::OnInitLiveWire (Action* action, const StateEvent* stateEvent)
{
  const PositionEvent* positionEvent = dynamic_cast<const PositionEvent*>(stateEvent->GetEvent());
  if (!positionEvent) return false;

  m_LastEventSender = positionEvent->GetSender();
  m_LastEventSlice = m_LastEventSender->GetSlice();

  if ( Superclass::CanHandleEvent(stateEvent) < 1.0 ) return false;

  int timestep = positionEvent->GetSender()->GetTimeStep();

  m_Contour = mitk::ContourModel::New();
  m_Contour->Expand(timestep+1);
  m_ContourModelNode = mitk::DataNode::New();
  m_ContourModelNode->SetData( m_Contour );
  m_ContourModelNode->SetName("working contour node");
  m_ContourModelNode->AddProperty( "contour.color", ColorProperty::New(1, 1, 0), NULL, true );
  m_ContourModelNode->AddProperty( "contour.points.color", ColorProperty::New(1.0, 0.0, 0.1), NULL, true );
  m_ContourModelNode->AddProperty( "contour.controlpoints.show", BoolProperty::New(true), NULL, true );

  m_LiveWireContour = mitk::ContourModel::New();
  m_LiveWireContour->Expand(timestep+1);
  m_LiveWireContourNode = mitk::DataNode::New();
  m_LiveWireContourNode->SetData( m_LiveWireContour );
  m_LiveWireContourNode->SetName("active livewire node");
  m_LiveWireContourNode->SetProperty( "layer", IntProperty::New(100));
  m_LiveWireContourNode->SetProperty( "helper object", mitk::BoolProperty::New(true));
  m_LiveWireContourNode->AddProperty( "contour.color", ColorProperty::New(0.1, 1.0, 0.1), NULL, true );
  m_LiveWireContourNode->AddProperty( "contour.width", mitk::FloatProperty::New( 4.0 ), NULL, true );

  m_EditingContour = mitk::ContourModel::New();
  m_EditingContour->Expand(timestep+1);
  m_EditingContourNode = mitk::DataNode::New();
  m_EditingContourNode->SetData( m_EditingContour );
  m_EditingContourNode->SetName("editing node");
  m_EditingContourNode->SetProperty( "layer", IntProperty::New(100));
  m_EditingContourNode->SetProperty( "helper object", mitk::BoolProperty::New(true));
  m_EditingContourNode->AddProperty( "contour.color", ColorProperty::New(0.1, 1.0, 0.1), NULL, true );
  m_EditingContourNode->AddProperty( "contour.points.color", ColorProperty::New(0.0, 0.0, 1.0), NULL, true );
  m_EditingContourNode->AddProperty( "contour.width", mitk::FloatProperty::New( 4.0 ), NULL, true );

  m_ToolManager->GetDataStorage()->Add( m_ContourModelNode );
  m_ToolManager->GetDataStorage()->Add( m_LiveWireContourNode );
  m_ToolManager->GetDataStorage()->Add( m_EditingContourNode );

  //set current slice as input for ImageToLiveWireContourFilter
  m_WorkingSlice = this->GetAffectedReferenceSlice(positionEvent);

  m_LiveWireFilter = mitk::ImageLiveWireContourModelFilter::New();
  m_LiveWireFilter->SetInput(m_WorkingSlice);

  //map click to pixel coordinates
  mitk::Point3D click = const_cast<mitk::Point3D &>(positionEvent->GetWorldPosition());
  itk::Index<3> idx;
  m_WorkingSlice->GetGeometry()->WorldToIndex(click, idx);

  // get the pixel the gradient in region of 5x5
  itk::Index<3> indexWithHighestGradient;
  AccessFixedDimensionByItk_2(m_WorkingSlice, FindHighestGradientMagnitudeByITK, 2, idx, indexWithHighestGradient);

  // itk::Index to mitk::Point3D
  click[0] = indexWithHighestGradient[0];
  click[1] = indexWithHighestGradient[1];
  click[2] = indexWithHighestGradient[2];
  m_WorkingSlice->GetGeometry()->IndexToWorld(click, click);

  //set initial start point
  m_Contour->AddVertex( click, true, timestep );
  m_LiveWireFilter->SetStartPoint(click);

  m_CreateAndUseDynamicCosts = true;

  //render
  assert( positionEvent->GetSender()->GetRenderWindow() );
  mitk::RenderingManager::GetInstance()->RequestUpdate( positionEvent->GetSender()->GetRenderWindow() );

  return true;
}

bool mitk::LiveWireTool2D::OnAddPoint (Action* action, const StateEvent* stateEvent)
{
  //complete LiveWire interaction for last segment
  //add current LiveWire contour to the finished contour and reset
  //to start new segment and computation

  /* check if event can be handled */
  const PositionEvent* positionEvent = dynamic_cast<const PositionEvent*>(stateEvent->GetEvent());
  if (!positionEvent) return false;

  if ( Superclass::CanHandleEvent(stateEvent) < 1.0 ) return false;
  /* END check if event can be handled */

  int timestep = positionEvent->GetSender()->GetTimeStep();

  //add repulsive points to avoid to get the same path again
  typedef mitk::ImageLiveWireContourModelFilter::InternalImageType::IndexType IndexType;
  mitk::ContourModel::ConstVertexIterator iter = m_LiveWireContour->IteratorBegin(timestep);
  for (;iter != m_LiveWireContour->IteratorEnd(timestep); iter++)
  {
      IndexType idx;
      this->m_WorkingSlice->GetGeometry()->WorldToIndex((*iter)->Coordinates, idx);

      this->m_LiveWireFilter->AddRepulsivePoint( idx );
  }

  //remove duplicate first vertex, it's already contained in m_Contour
  m_LiveWireContour->RemoveVertexAt(0, timestep);

  // set last added point as control point
  m_LiveWireContour->SetControlVertexAt(m_LiveWireContour->GetNumberOfVertices(timestep)-1, timestep);

  //merge contours
  m_Contour->Concatenate(m_LiveWireContour, timestep);

  //clear the livewire contour and reset the corresponding datanode
  m_LiveWireContour->Clear(timestep);

  //set new start point
  m_LiveWireFilter->SetStartPoint(const_cast<mitk::Point3D &>(positionEvent->GetWorldPosition()));

  if( m_CreateAndUseDynamicCosts )
  {
    //use dynamic cost map for next update
    m_LiveWireFilter->CreateDynamicCostMap(m_Contour);
    m_LiveWireFilter->SetUseDynamicCostMap(true);
    //m_CreateAndUseDynamicCosts = false;
  }

  //render
  assert( positionEvent->GetSender()->GetRenderWindow() );
  mitk::RenderingManager::GetInstance()->RequestUpdate( positionEvent->GetSender()->GetRenderWindow() );

  return true;
}

bool mitk::LiveWireTool2D::OnMouseMoved( Action* action, const StateEvent* stateEvent)
{
  //compute LiveWire segment from last control point to current mouse position

  // check if event can be handled
  if ( Superclass::CanHandleEvent(stateEvent) < 1.0 ) return false;

  const PositionEvent* positionEvent = dynamic_cast<const PositionEvent*>(stateEvent->GetEvent());
  if (!positionEvent) return false;

  // actual LiveWire computation
  int timestep = positionEvent->GetSender()->GetTimeStep();

  m_LiveWireFilter->SetEndPoint(const_cast<mitk::Point3D &>(positionEvent->GetWorldPosition()));

  m_LiveWireFilter->SetTimeStep(m_TimeStep);
  m_LiveWireFilter->Update();

  m_LiveWireContour = this->m_LiveWireFilter->GetOutput();
  m_LiveWireContourNode->SetData( this->m_LiveWireContour );

  //render
  assert( positionEvent->GetSender()->GetRenderWindow() );
  mitk::RenderingManager::GetInstance()->RequestUpdate( positionEvent->GetSender()->GetRenderWindow() );

  return true;
}

bool mitk::LiveWireTool2D::OnMouseMoveNoDynamicCosts(Action* action, const StateEvent* stateEvent)
{
  //do not use dynamic cost map
  m_LiveWireFilter->SetUseDynamicCostMap(false);
  OnMouseMoved(action, stateEvent);
  m_LiveWireFilter->SetUseDynamicCostMap(true);
  return true;
}

bool mitk::LiveWireTool2D::OnCheckPoint( Action* action, const StateEvent* stateEvent)
{
  //check double click on first control point to finish the LiveWire tool
  //
  //Check distance to first point.
  //Transition YES if click close to first control point
  //

  mitk::StateEvent* newStateEvent = NULL;

  const PositionEvent* positionEvent = dynamic_cast<const PositionEvent*>(stateEvent->GetEvent());
  if (!positionEvent)
  {
    //stay in current state
    newStateEvent = new mitk::StateEvent(EIDNO, stateEvent->GetEvent());
  }
  else
  {
    int timestep = positionEvent->GetSender()->GetTimeStep();

    mitk::Point3D click = positionEvent->GetWorldPosition();

    mitk::Point3D first = this->m_Contour->GetVertexAt(0, timestep)->Coordinates;

    if (first.EuclideanDistanceTo(click) < 4.5)
    {
      // allow to finish
      newStateEvent = new mitk::StateEvent(EIDYES, stateEvent->GetEvent());
    }
    else
    {
      //stay active
      newStateEvent = new mitk::StateEvent(EIDNO, stateEvent->GetEvent());
    }
  }

  this->HandleEvent( newStateEvent );

  return true;
}

bool mitk::LiveWireTool2D::OnFinish( Action* action, const StateEvent* stateEvent)
{
  // finish livewire tool interaction

  // check if event can be handled
  if ( Superclass::CanHandleEvent(stateEvent) < 1.0 ) return false;

  const PositionEvent* positionEvent = dynamic_cast<const PositionEvent*>(stateEvent->GetEvent());
  if (!positionEvent) return false;

  // actual timestep
  int timestep = positionEvent->GetSender()->GetTimeStep();

  // remove last control point being added by double click
  m_Contour->RemoveVertexAt(m_Contour->GetNumberOfVertices(timestep) - 1, timestep);

  // save contour and corresponding plane geometry to list
  std::pair<mitk::DataNode::Pointer, mitk::PlaneGeometry::Pointer> cp(m_ContourModelNode, dynamic_cast<mitk::PlaneGeometry*>(positionEvent->GetSender()->GetCurrentWorldGeometry2D()->Clone().GetPointer()) );
  this->m_WorkingContours.push_back(cp);

  std::pair<mitk::DataNode::Pointer, mitk::PlaneGeometry::Pointer> ecp(m_EditingContourNode, dynamic_cast<mitk::PlaneGeometry*>(positionEvent->GetSender()->GetCurrentWorldGeometry2D()->Clone().GetPointer()) );
  this->m_EditingContours.push_back(ecp);

  m_LiveWireFilter->SetUseDynamicCostMap(false);

  this->FinishTool();

  return true;
}

void mitk::LiveWireTool2D::FinishTool()
{
  unsigned int numberOfTimesteps = m_Contour->GetTimeSlicedGeometry()->GetTimeSteps();

  //close contour in each timestep
  for( int i = 0; i <= numberOfTimesteps; i++)
  {
    m_Contour->Close(i);
  }

  m_ToolManager->GetDataStorage()->Remove( m_LiveWireContourNode );

  // clear live wire contour node
  m_LiveWireContourNode = NULL;
  m_LiveWireContour = NULL;

  //change color as visual feedback of completed livewire
  //m_ContourModelNode->AddProperty( "contour.color", ColorProperty::New(1.0, 1.0, 0.1), NULL, true );
  //m_ContourModelNode->SetName("contour node");

  //set the livewire interactor to edit control points
  m_ContourInteractor = mitk::ContourModelLiveWireInteractor::New(m_ContourModelNode);
  m_ContourInteractor->SetWorkingImage(this->m_WorkingSlice);
  m_ContourInteractor->SetEditingContourModelNode(this->m_EditingContourNode);
  m_ContourModelNode->SetInteractor(m_ContourInteractor);

  this->m_LiveWireInteractors.push_back( m_ContourInteractor );

  //add interactor to globalInteraction instance
  mitk::GlobalInteraction::GetInstance()->AddInteractor(m_ContourInteractor);
}

bool mitk::LiveWireTool2D::OnLastSegmentDelete( Action* action, const StateEvent* stateEvent)
{
  int timestep = stateEvent->GetEvent()->GetSender()->GetTimeStep();

  //if last point of current contour will be removed go to start state and remove nodes
  if( m_Contour->GetNumberOfVertices(timestep) <= 1 )
  {
    m_ToolManager->GetDataStorage()->Remove( m_LiveWireContourNode );
    m_ToolManager->GetDataStorage()->Remove( m_ContourModelNode );
    m_ToolManager->GetDataStorage()->Remove( m_EditingContourNode );
    m_LiveWireContour = mitk::ContourModel::New();
    m_Contour = mitk::ContourModel::New();
    m_ContourModelNode->SetData( m_Contour );
    m_LiveWireContourNode->SetData( m_LiveWireContour );
    Superclass::Deactivated(); //go to start state
  }
  else //remove last segment from contour and reset livewire contour
  {

    m_LiveWireContour = mitk::ContourModel::New();

    m_LiveWireContourNode->SetData(m_LiveWireContour);


    mitk::ContourModel::Pointer newContour = mitk::ContourModel::New();
    newContour->Expand(m_Contour->GetTimeSteps());

    mitk::ContourModel::VertexIterator begin = m_Contour->IteratorBegin();

    //iterate from last point to next active point
    mitk::ContourModel::VertexIterator newLast = m_Contour->IteratorBegin() + (m_Contour->GetNumberOfVertices() - 1);

    //go at least one down
    if(newLast != begin)
    {
      newLast--;
    }

    //search next active control point
    while(newLast != begin && !((*newLast)->IsControlPoint) )
    {
      newLast--;
    }

    //set position of start point for livewire filter to coordinates of the new last point
    m_LiveWireFilter->SetStartPoint((*newLast)->Coordinates);

    mitk::ContourModel::VertexIterator it = m_Contour->IteratorBegin();

    //fill new Contour
    while(it <= newLast)
    {
      newContour->AddVertex((*it)->Coordinates, (*it)->IsControlPoint, timestep);
      it++;
    }

    newContour->SetIsClosed(m_Contour->IsClosed());

    //set new contour visible
    m_ContourModelNode->SetData(newContour);

    m_Contour = newContour;

    assert( stateEvent->GetEvent()->GetSender()->GetRenderWindow() );
    mitk::RenderingManager::GetInstance()->RequestUpdate( stateEvent->GetEvent()->GetSender()->GetRenderWindow() );
  }

  return true;
}


template<typename TPixel, unsigned int VImageDimension>
void mitk::LiveWireTool2D::FindHighestGradientMagnitudeByITK(itk::Image<TPixel, VImageDimension>* inputImage, itk::Index<3> &index, itk::Index<3> &returnIndex)
{
  typedef itk::Image<TPixel, VImageDimension>  InputImageType;
  typedef typename InputImageType::IndexType            IndexType;

  unsigned long xMAX = inputImage->GetLargestPossibleRegion().GetSize()[0];
  unsigned long yMAX = inputImage->GetLargestPossibleRegion().GetSize()[1];

  returnIndex[0] = index[0];
  returnIndex[1] = index[1];
  returnIndex[2] = 0.0;


  double gradientMagnitude = 0.0;
  double maxGradientMagnitude = 0.0;

  /*
    the size and thus the region of 7x7 is only used to calculate the gradient magnitude in that region
    not for searching the maximum value
    */

  //maximum value in each direction for size
  typename InputImageType::SizeType size;
  size[0] = 7;
  size[1] = 7;

  //minimum value in each direction for startRegion
  IndexType startRegion;
  startRegion[0] = index[0] - 3;
  startRegion[1] = index[1] - 3;
  if(startRegion[0] < 0) startRegion[0] = 0;
  if(startRegion[1] < 0) startRegion[1] = 0;
  if(xMAX - index[0] < 7) startRegion[0] = xMAX - 7;
  if(yMAX - index[1] < 7) startRegion[1] = yMAX - 7;

  index[0] = startRegion[0] + 3;
  index[1] = startRegion[1] + 3;



  typename InputImageType::RegionType region;
  region.SetSize( size );
  region.SetIndex( startRegion );

  typedef typename  itk::GradientMagnitudeImageFilter< InputImageType, InputImageType> GradientMagnitudeFilterType;
  typename GradientMagnitudeFilterType::Pointer gradientFilter = GradientMagnitudeFilterType::New();
  gradientFilter->SetInput(inputImage);
  gradientFilter->GetOutput()->SetRequestedRegion(region);

  gradientFilter->Update();
  typename InputImageType::Pointer gradientMagnImage;
  gradientMagnImage = gradientFilter->GetOutput();


  IndexType currentIndex;
  currentIndex[0] = 0;
  currentIndex[1] = 0;

  // search max (approximate) gradient magnitude
  for( int x = -1; x <= 1; ++x)
  {
    currentIndex[0] = index[0] + x;

    for( int y = -1; y <= 1; ++y)
    {
      currentIndex[1] = index[1] + y;

      gradientMagnitude = gradientMagnImage->GetPixel(currentIndex);

      //check for new max
      if(maxGradientMagnitude < gradientMagnitude)
      {
        maxGradientMagnitude = gradientMagnitude;
        returnIndex[0] = currentIndex[0];
        returnIndex[1] = currentIndex[1];
        returnIndex[2] = 0.0;
      }//end if
    }//end for y

    currentIndex[1] = index[1];
  }//end for x

}
