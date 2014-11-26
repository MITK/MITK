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

#include <itkGradientMagnitudeImageFilter.h>

#include <mitkBaseRenderer.h>
#include <mitkContour.h>
#include <mitkContourModelUtils.h>
#include <mitkContourUtils.h>
#include <mitkGlobalInteraction.h>
#include <mitkInteractionConst.h>
#include <mitkRenderingManager.h>
#include <mitkToolManager.h>

#include <usGetModuleContext.h>
#include <usModule.h>
#include <usModuleContext.h>
#include <usModuleResource.h>

#include "mitkLiveWireTool2D.h"
#include "mitkLiveWireTool2D.xpm"

namespace mitk
{
  MITK_TOOL_MACRO(MitkSegmentation_EXPORT, LiveWireTool2D, "LiveWire tool");
}

static void AddInteractorToGlobalInteraction(mitk::Interactor* interactor)
{
  mitk::GlobalInteraction::GetInstance()->AddInteractor(interactor);
}

static void RemoveInteractorFromGlobalInteraction(mitk::Interactor* interactor)
{
  mitk::GlobalInteraction::GetInstance()->RemoveInteractor(interactor);
}

class RemoveFromDataStorage
{
public:
  RemoveFromDataStorage(mitk::DataStorage::Pointer dataStorage)
    : m_DataStorage(dataStorage)
  {
  }

  void operator()(mitk::DataNode* dataNode)
  {
    m_DataStorage->Remove(dataNode);
  }

  void operator()(const std::pair<mitk::DataNode::Pointer, mitk::PlaneGeometry::Pointer>& dataNode)
  {
    m_DataStorage->Remove(dataNode.first);
  }

private:
  mitk::DataStorage::Pointer m_DataStorage;
};

mitk::LiveWireTool2D::LiveWireTool2D()
  : SegTool2D("LiveWireTool"), m_PlaneGeometry(NULL)
{
}

mitk::LiveWireTool2D::~LiveWireTool2D()
{
  this->ClearSegmentation();
}

void mitk::LiveWireTool2D::RemoveHelperObjects()
{
  DataStorage* dataStorage = m_ToolManager->GetDataStorage();

  if (!m_EditingContours.empty())
    std::for_each(m_EditingContours.begin(), m_EditingContours.end(), RemoveFromDataStorage(dataStorage));

  if (!m_WorkingContours.empty())
    std::for_each(m_WorkingContours.begin(), m_WorkingContours.end(), RemoveFromDataStorage(dataStorage));

  if (m_EditingContourNode.IsNotNull())
    dataStorage->Remove(m_EditingContourNode);

  if (m_LiveWireContourNode.IsNotNull())
    dataStorage->Remove(m_LiveWireContourNode);

  if (m_ContourModelNode.IsNotNull())
    dataStorage->Remove(m_ContourModelNode);

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void mitk::LiveWireTool2D::ReleaseHelperObjects()
{
  this->RemoveHelperObjects();

  if (!m_EditingContours.empty())
    m_EditingContours.clear();

  if (!m_WorkingContours.empty())
    m_WorkingContours.clear();

  m_EditingContourNode = NULL;
  m_EditingContour = NULL;

  m_LiveWireContourNode = NULL;
  m_LiveWireContour = NULL;

  m_ContourModelNode = NULL;
  m_Contour = NULL;
}

void mitk::LiveWireTool2D::ReleaseInteractors()
{
  this->EnableContourLiveWireInteraction(false);
  m_LiveWireInteractors.clear();
}

void mitk::LiveWireTool2D::ConnectActionsAndFunctions()
{
  CONNECT_CONDITION("CheckContourClosed", OnCheckPoint);

  CONNECT_FUNCTION("InitObject", OnInitLiveWire);
  CONNECT_FUNCTION("AddPoint", OnAddPoint);
  CONNECT_FUNCTION("CtrlAddPoint", OnAddPoint);
  CONNECT_FUNCTION("MovePoint", OnMouseMoveNoDynamicCosts);
  CONNECT_FUNCTION("FinishContour", OnFinish);
  CONNECT_FUNCTION("DeletePoint", OnLastSegmentDelete);
  CONNECT_FUNCTION("CtrlMovePoint", OnMouseMoved);
}

const char** mitk::LiveWireTool2D::GetXPM() const
{
  return mitkLiveWireTool2D_xpm;
}

us::ModuleResource mitk::LiveWireTool2D::GetIconResource() const
{
  return us::GetModuleContext()->GetModule()->GetResource("LiveWire_48x48.png");
}

us::ModuleResource mitk::LiveWireTool2D::GetCursorIconResource() const
{
  return us::GetModuleContext()->GetModule()->GetResource("LiveWire_Cursor_32x32.png");
}

const char* mitk::LiveWireTool2D::GetName() const
{
  return "Live Wire";
}

void mitk::LiveWireTool2D::Activated()
{
  Superclass::Activated();
  this->ResetToStartState();
  this->EnableContourLiveWireInteraction(true);
}

void mitk::LiveWireTool2D::Deactivated()
{
  Superclass::Deactivated();
  this->ConfirmSegmentation();
}

void mitk::LiveWireTool2D::EnableContourLiveWireInteraction(bool on)
{
  std::for_each(m_LiveWireInteractors.begin(), m_LiveWireInteractors.end(), on
                ? AddInteractorToGlobalInteraction
                : RemoveInteractorFromGlobalInteraction);
}

void mitk::LiveWireTool2D::ConfirmSegmentation()
{
  DataNode* workingNode( m_ToolManager->GetWorkingData(0) );

  if (!workingNode)
    return;

  Image* workingImage = dynamic_cast<Image*>(workingNode->GetData());

  if (!workingImage)
    return;

  // for all contours in list (currently created by tool)
  std::vector< std::pair<mitk::DataNode::Pointer, mitk::PlaneGeometry::Pointer> >::iterator itWorkingContours = this->m_WorkingContours.begin();
  std::vector<SliceInformation> sliceList;
  sliceList.reserve(m_WorkingContours.size());
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
        for( TimeStepType currentTimestep = 0; currentTimestep < contourModel->GetTimeGeometry()->CountTimeSteps(); ++currentTimestep)
        {

          //get the segmentation image slice at current timestep
          mitk::Image::Pointer workingSlice = this->GetAffectedImageSliceAs2DImage(itWorkingContours->second, workingImage, currentTimestep);

          mitk::ContourModel::Pointer projectedContour = mitk::ContourModelUtils::ProjectContourTo2DSlice(workingSlice, contourModel, true, false);
          mitk::ContourModelUtils::FillContourInSlice(projectedContour, workingSlice, 1.0);

          //write back to image volume
          SliceInformation sliceInfo (workingSlice, itWorkingContours->second, currentTimestep);
          sliceList.push_back(sliceInfo);
          this->WriteSliceToVolume(sliceInfo);
        }
      }
    }
    ++itWorkingContours;
  }

  this->WriteBackSegmentationResult(sliceList, false);
  this->ClearSegmentation();
}

void mitk::LiveWireTool2D::ClearSegmentation()
{
  this->ReleaseHelperObjects();
  this->ReleaseInteractors();
  this->ResetToStartState();
}

bool mitk::LiveWireTool2D::OnInitLiveWire ( StateMachineAction*, InteractionEvent* interactionEvent )
{
  mitk::InteractionPositionEvent* positionEvent = dynamic_cast<mitk::InteractionPositionEvent*>( interactionEvent );

  if (!positionEvent) return false;

  m_LastEventSender = positionEvent->GetSender();
  m_LastEventSlice = m_LastEventSender->GetSlice();

  int timestep = positionEvent->GetSender()->GetTimeStep();

  m_Contour = mitk::ContourModel::New();
  m_Contour->Expand(timestep+1);
  m_ContourModelNode = mitk::DataNode::New();
  m_ContourModelNode->SetData( m_Contour );
  m_ContourModelNode->SetName("working contour node");
  m_ContourModelNode->SetProperty( "layer", IntProperty::New(100));
  m_ContourModelNode->AddProperty( "fixedLayer", BoolProperty::New(true));
  m_ContourModelNode->SetProperty( "helper object", mitk::BoolProperty::New(true));
  m_ContourModelNode->AddProperty( "contour.color", ColorProperty::New(1, 1, 0), NULL, true );
  m_ContourModelNode->AddProperty( "contour.points.color", ColorProperty::New(1.0, 0.0, 0.1), NULL, true );
  m_ContourModelNode->AddProperty( "contour.controlpoints.show", BoolProperty::New(true), NULL, true );

  m_LiveWireContour = mitk::ContourModel::New();
  m_LiveWireContour->Expand(timestep+1);
  m_LiveWireContourNode = mitk::DataNode::New();
  m_LiveWireContourNode->SetData( m_LiveWireContour );
  m_LiveWireContourNode->SetName("active livewire node");
  m_LiveWireContourNode->SetProperty( "layer", IntProperty::New(101));
  m_LiveWireContourNode->AddProperty( "fixedLayer", BoolProperty::New(true));
  m_LiveWireContourNode->SetProperty( "helper object", mitk::BoolProperty::New(true));
  m_LiveWireContourNode->AddProperty( "contour.color", ColorProperty::New(0.1, 1.0, 0.1), NULL, true );
  m_LiveWireContourNode->AddProperty( "contour.width", mitk::FloatProperty::New( 4.0 ), NULL, true );

  m_EditingContour = mitk::ContourModel::New();
  m_EditingContour->Expand(timestep+1);
  m_EditingContourNode = mitk::DataNode::New();
  m_EditingContourNode->SetData( m_EditingContour );
  m_EditingContourNode->SetName("editing node");
  m_EditingContourNode->SetProperty( "layer", IntProperty::New(102));
  m_EditingContourNode->AddProperty( "fixedLayer", BoolProperty::New(true));
  m_EditingContourNode->SetProperty( "helper object", mitk::BoolProperty::New(true));
  m_EditingContourNode->AddProperty( "contour.color", ColorProperty::New(0.1, 1.0, 0.1), NULL, true );
  m_EditingContourNode->AddProperty( "contour.points.color", ColorProperty::New(0.0, 0.0, 1.0), NULL, true );
  m_EditingContourNode->AddProperty( "contour.width", mitk::FloatProperty::New( 4.0 ), NULL, true );

  mitk::DataNode* workingDataNode = m_ToolManager->GetWorkingData(0);

  m_ToolManager->GetDataStorage()->Add(m_ContourModelNode, workingDataNode);
  m_ToolManager->GetDataStorage()->Add(m_LiveWireContourNode, workingDataNode);
  m_ToolManager->GetDataStorage()->Add(m_EditingContourNode, workingDataNode);

  //set current slice as input for ImageToLiveWireContourFilter
  m_WorkingSlice = this->GetAffectedReferenceSlice(positionEvent);

  //Transfer LiveWire's center based contour output to corner based via the adaption of the input
  //slice image. Just in case someone stumbles across the 0.5 here I know what I'm doing ;-).
  mitk::Point3D newOrigin = m_WorkingSlice->GetSlicedGeometry()->GetOrigin();
  m_WorkingSlice->GetSlicedGeometry()->WorldToIndex(newOrigin, newOrigin);
  newOrigin[0] -= 0.5;
  newOrigin[1] -= 0.5;
  m_WorkingSlice->GetSlicedGeometry()->IndexToWorld(newOrigin, newOrigin);
  m_WorkingSlice->GetSlicedGeometry()->SetOrigin(newOrigin);


  m_LiveWireFilter = mitk::ImageLiveWireContourModelFilter::New();
  m_LiveWireFilter->SetInput(m_WorkingSlice);

  //map click to pixel coordinates
  mitk::Point3D click = positionEvent->GetPositionInWorld();
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
  // remember plane geometry to determine if events were triggered in same plane
  m_PlaneGeometry = interactionEvent->GetSender()->GetCurrentWorldPlaneGeometry();

  m_CreateAndUseDynamicCosts = true;

  //render
  assert( positionEvent->GetSender()->GetRenderWindow() );
  mitk::RenderingManager::GetInstance()->RequestUpdate( positionEvent->GetSender()->GetRenderWindow() );

  return true;
}


bool mitk::LiveWireTool2D::OnAddPoint ( StateMachineAction*, InteractionEvent* interactionEvent )
{
  //complete LiveWire interaction for last segment
  //add current LiveWire contour to the finished contour and reset
  //to start new segment and computation

  mitk::InteractionPositionEvent* positionEvent = dynamic_cast<mitk::InteractionPositionEvent*>( interactionEvent );
  if (!positionEvent) return false;

  if (m_PlaneGeometry != NULL)
  {
    // this checks that the point is in the correct slice
    if (m_PlaneGeometry->DistanceFromPlane(positionEvent->GetPositionInWorld()) > mitk::eps)
      return false;
  }

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
  m_LiveWireFilter->SetStartPoint(positionEvent->GetPositionInWorld());

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

bool mitk::LiveWireTool2D::OnMouseMoved( StateMachineAction*, InteractionEvent* interactionEvent )
{
  //compute LiveWire segment from last control point to current mouse position

  mitk::InteractionPositionEvent* positionEvent = dynamic_cast<mitk::InteractionPositionEvent*>( interactionEvent );
  if (!positionEvent) return false;

  // actual LiveWire computation
  int timestep = positionEvent->GetSender()->GetTimeStep();

  m_LiveWireFilter->SetEndPoint(positionEvent->GetPositionInWorld());

  m_LiveWireFilter->SetTimeStep( timestep );
  m_LiveWireFilter->Update();

  m_LiveWireContour = this->m_LiveWireFilter->GetOutput();
  m_LiveWireContourNode->SetData( this->m_LiveWireContour );

  //render
  assert( positionEvent->GetSender()->GetRenderWindow() );
  positionEvent->GetSender()->GetRenderingManager()->RequestUpdate( positionEvent->GetSender()->GetRenderWindow() );

  return true;
}


bool mitk::LiveWireTool2D::OnMouseMoveNoDynamicCosts( StateMachineAction*, InteractionEvent* interactionEvent )
{
  //do not use dynamic cost map
  m_LiveWireFilter->SetUseDynamicCostMap(false);
  OnMouseMoved( NULL, interactionEvent);
  m_LiveWireFilter->SetUseDynamicCostMap(true);
  return true;
}


bool mitk::LiveWireTool2D::OnCheckPoint( const InteractionEvent* interactionEvent)
{
  //check double click on first control point to finish the LiveWire tool
  //
  //Check distance to first point.
  //Transition YES if click close to first control point
  //

  const mitk::InteractionPositionEvent* positionEvent = dynamic_cast<const mitk::InteractionPositionEvent*>( interactionEvent );
  if (positionEvent)
  {
    int timestep = positionEvent->GetSender()->GetTimeStep();

    mitk::Point3D click = positionEvent->GetPositionInWorld();
    mitk::Point3D first = this->m_Contour->GetVertexAt(0, timestep)->Coordinates;

    if (first.EuclideanDistanceTo(click) < 4.5)
    {
      // allow to finish
      return true;
    }
    else
    {
      return false;
    }
  }

  return false;
}


bool mitk::LiveWireTool2D::OnFinish( StateMachineAction*, InteractionEvent* interactionEvent )
{
  // finish livewire tool interaction

  mitk::InteractionPositionEvent* positionEvent = dynamic_cast<mitk::InteractionPositionEvent*>( interactionEvent );
  if (!positionEvent) return false;

  // Have to do that here so that the m_LastEventSender is set correctly
  mitk::SegTool2D::AddContourmarker();

  // actual timestep
  int timestep = positionEvent->GetSender()->GetTimeStep();

  // remove last control point being added by double click
  m_Contour->RemoveVertexAt(m_Contour->GetNumberOfVertices(timestep) - 1, timestep);

  // save contour and corresponding plane geometry to list
  std::pair<mitk::DataNode::Pointer, mitk::PlaneGeometry::Pointer> cp(m_ContourModelNode, dynamic_cast<mitk::PlaneGeometry*>(positionEvent->GetSender()->GetCurrentWorldPlaneGeometry()->Clone().GetPointer()) );
  this->m_WorkingContours.push_back(cp);

  std::pair<mitk::DataNode::Pointer, mitk::PlaneGeometry::Pointer> ecp(m_EditingContourNode, dynamic_cast<mitk::PlaneGeometry*>(positionEvent->GetSender()->GetCurrentWorldPlaneGeometry()->Clone().GetPointer()) );
  this->m_EditingContours.push_back(ecp);

  m_LiveWireFilter->SetUseDynamicCostMap(false);

  this->FinishTool();

  return true;
}

void mitk::LiveWireTool2D::FinishTool()
{
  TimeStepType numberOfTimesteps = m_Contour->GetTimeGeometry()->CountTimeSteps();

  //close contour in each timestep
  for( int i = 0; i <= numberOfTimesteps; i++)
  {
    m_Contour->Close(i);
  }

  m_ToolManager->GetDataStorage()->Remove( m_LiveWireContourNode );

  // clear live wire contour node
  m_LiveWireContourNode = NULL;
  m_LiveWireContour = NULL;

  //set the livewire interactor to edit control points
  m_ContourInteractor = mitk::ContourModelLiveWireInteractor::New(m_ContourModelNode);
  m_ContourInteractor->SetWorkingImage(this->m_WorkingSlice);
  m_ContourInteractor->SetEditingContourModelNode(this->m_EditingContourNode);
  m_ContourModelNode->SetInteractor(m_ContourInteractor);

  this->m_LiveWireInteractors.push_back( m_ContourInteractor );

  //add interactor to globalInteraction instance
  mitk::GlobalInteraction::GetInstance()->AddInteractor(m_ContourInteractor);
}


bool mitk::LiveWireTool2D::OnLastSegmentDelete( StateMachineAction*, InteractionEvent* interactionEvent )
{
  int timestep = interactionEvent->GetSender()->GetTimeStep();

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
    this->ResetToStartState(); //go to start state
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

    newContour->SetClosed(m_Contour->IsClosed());

    //set new contour visible
    m_ContourModelNode->SetData(newContour);

    m_Contour = newContour;

    assert( interactionEvent->GetSender()->GetRenderWindow() );
    mitk::RenderingManager::GetInstance()->RequestUpdate( interactionEvent->GetSender()->GetRenderWindow() );
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
