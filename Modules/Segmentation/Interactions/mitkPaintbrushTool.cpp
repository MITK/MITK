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

#include "mitkPaintbrushTool.h"

#include "mitkToolManager.h"
#include "mitkOverwriteSliceImageFilter.h"
#include "mitkBaseRenderer.h"
#include "mitkImageDataItem.h"
#include "ipSegmentation.h"
#include "mitkAbstractTransformGeometry.h"

#include "mitkLevelWindowProperty.h"

#define ROUND(a)     ((a)>0 ? (int)((a)+0.5) : -(int)(0.5-(a)))

int mitk::PaintbrushTool::m_Size = 1;

mitk::PaintbrushTool::PaintbrushTool(int paintingPixelValue)
:FeedbackContourTool("PressMoveReleaseWithCTRLInversionAllMouseMoves"),
 m_PaintingPixelValue(paintingPixelValue),
 m_LastContourSize(0) // other than initial mitk::PaintbrushTool::m_Size (around l. 28)
{
  m_MasterContour = ContourModel::New();
  m_MasterContour->Initialize();
  m_CurrentPlane = NULL;

  m_WorkingNode = DataNode::New();
  m_WorkingNode->SetProperty( "levelwindow", mitk::LevelWindowProperty::New( mitk::LevelWindow(0, 1) ) );
  m_WorkingNode->SetProperty( "binary", mitk::BoolProperty::New(true) );
}

mitk::PaintbrushTool::~PaintbrushTool()
{
}

void mitk::PaintbrushTool::ConnectActionsAndFunctions()
{
  CONNECT_FUNCTION( "PrimaryButtonPressed", OnMousePressed);
  CONNECT_FUNCTION( "Move", OnPrimaryButtonPressedMoved);
  CONNECT_FUNCTION( "MouseMove", OnMouseMoved);
  CONNECT_FUNCTION( "Release", OnMouseReleased);
  CONNECT_FUNCTION( "InvertLogic", OnInvertLogic);
}

void mitk::PaintbrushTool::Activated()
{
  Superclass::Activated();
  FeedbackContourTool::SetFeedbackContourVisible(true);
  SizeChanged.Send(m_Size);
  m_ToolManager->WorkingDataChanged += mitk::MessageDelegate<mitk::PaintbrushTool>( this, &mitk::PaintbrushTool::OnToolManagerWorkingDataModified );
}

void mitk::PaintbrushTool::Deactivated()
{
  FeedbackContourTool::SetFeedbackContourVisible(false);
  if (m_ToolManager->GetDataStorage()->Exists(m_WorkingNode))
      m_ToolManager->GetDataStorage()->Remove(m_WorkingNode);
  Superclass::Deactivated();
  m_WorkingSlice = NULL;
  m_CurrentPlane = NULL;
  m_ToolManager->WorkingDataChanged -= mitk::MessageDelegate<mitk::PaintbrushTool>( this, &mitk::PaintbrushTool::OnToolManagerWorkingDataModified );
}

void mitk::PaintbrushTool::SetSize(int value)
{
  m_Size = value;
}

mitk::Point2D mitk::PaintbrushTool::upperLeft(mitk::Point2D p)
{
   p[0] -= 0.5;
   p[1] += 0.5;
   return p;
}


void mitk::PaintbrushTool::UpdateContour(const InteractionPositionEvent* positionEvent)
{
  //MITK_INFO<<"Update...";
  // examine stateEvent and create a contour that matches the pixel mask that we are going to draw
  //mitk::InteractionPositionEvent* positionEvent = dynamic_cast<mitk::InteractionPositionEvent*>( interactionEvent );
  //const PositionEvent* positionEvent = dynamic_cast<const PositionEvent*>(stateEvent->GetEvent());
  if (!positionEvent) return;

  // Get Spacing of current Slice
  //mitk::Vector3D vSpacing = m_WorkingSlice->GetSlicedGeometry()->GetPlaneGeometry(0)->GetSpacing();

  //
  // Draw a contour in Square according to selected brush size
  //
  int radius = (m_Size)/2;
  float fradius = static_cast<float>(m_Size) / 2.0f;

  ContourModel::Pointer contourInImageIndexCoordinates = ContourModel::New();


  // estimate center point of the brush ( relative to the pixel the mouse points on )
  // -- left upper corner for even sizes,
  // -- midpoint for uneven sizes
  mitk::Point2D centerCorrection;
  centerCorrection.Fill(0);

  // even --> correction of [+0.5, +0.5]
  bool evenSize = ((m_Size % 2) == 0);
  if( evenSize )
  {
    centerCorrection[0] += 0.5;
    centerCorrection[1] += 0.5;
  }

  // we will compute the control points for the upper left quarter part of a circle contour
  std::vector< mitk::Point2D > quarterCycleUpperRight;
  std::vector< mitk::Point2D > quarterCycleLowerRight;
  std::vector< mitk::Point2D > quarterCycleLowerLeft;
  std::vector< mitk::Point2D > quarterCycleUpperLeft;


  mitk::Point2D curPoint;
  bool curPointIsInside = true;
  curPoint[0] = 0;
  curPoint[1] = radius;
  quarterCycleUpperRight.push_back( upperLeft(curPoint) );

  // to estimate if a pixel is inside the circle, we need to compare against the 'outer radius'
  // i.e. the distance from the midpoint [0,0] to the border of the pixel [0,radius]
  //const float outer_radius = static_cast<float>(radius) + 0.5;

  while (curPoint[1] > 0)
  {
     // Move right until pixel is outside circle
     float curPointX_squared = 0.0f;
     float curPointY_squared = (curPoint[1] - centerCorrection[1] ) * (curPoint[1] - centerCorrection[1] );
     while( curPointIsInside )
     {
        // increment posX and chec
        curPoint[0]++;
        curPointX_squared = (curPoint[0] - centerCorrection[0] ) * (curPoint[0] - centerCorrection[0] );
        const float len = sqrt( curPointX_squared + curPointY_squared);
        if ( len > fradius )
        {
           // found first Pixel in this horizontal line, that is outside the circle
           curPointIsInside = false;
        }
     }
     quarterCycleUpperRight.push_back( upperLeft(curPoint) );

     // Move down until pixel is inside circle
     while( !curPointIsInside )
     {
        // increment posX and chec
        curPoint[1]--;
        curPointY_squared = (curPoint[1] - centerCorrection[1] ) * (curPoint[1] - centerCorrection[1] );
        const float len = sqrt( curPointX_squared + curPointY_squared);
        if ( len <= fradius )
        {
           // found first Pixel in this horizontal line, that is outside the circle
           curPointIsInside = true;
           quarterCycleUpperRight.push_back( upperLeft(curPoint) );
        }

        // Quarter cycle is full, when curPoint y position is 0
        if (curPoint[1] <= 0)
           break;
     }

   }

  // QuarterCycle is full! Now copy quarter cycle to other quarters.

  if( !evenSize )
  {
    std::vector< mitk::Point2D >::const_iterator it = quarterCycleUpperRight.begin();
    while( it != quarterCycleUpperRight.end() )
    {
      mitk::Point2D p;
      p = *it;

      // the contour points in the lower right corner have same position but with negative y values
      p[1] *= -1;
      quarterCycleLowerRight.push_back(p);

      // the contour points in the lower left corner have same position
      // but with both x,y negative
      p[0] *= -1;
      quarterCycleLowerLeft.push_back(p);

      // the contour points in the upper left corner have same position
      // but with x negative
      p[1] *= -1;
      quarterCycleUpperLeft.push_back(p);

      it++;
    }
  }
  else
  {
    std::vector< mitk::Point2D >::const_iterator it = quarterCycleUpperRight.begin();
    while( it != quarterCycleUpperRight.end() )
    {
      mitk::Point2D p,q;
      p = *it;

      q = p;
      // the contour points in the lower right corner have same position but with negative y values
      q[1] *= -1;
      // correct for moved offset if size even = the midpoint is not the midpoint of the current pixel
      // but its upper rigt corner
      q[1] += 1;
      quarterCycleLowerRight.push_back(q);

      q = p;
      // the contour points in the lower left corner have same position
      // but with both x,y negative
      q[1] = -1.0f * q[1] + 1;
      q[0] = -1.0f * q[0] + 1;
      quarterCycleLowerLeft.push_back(q);

      // the contour points in the upper left corner have same position
      // but with x negative
      q = p;
      q[0] *= -1;
      q[0] +=  1;
      quarterCycleUpperLeft.push_back(q);

      it++;
    }
  }

  // fill contour with poins in right ordering, starting with the upperRight block
  mitk::Point3D tempPoint;
  for (unsigned int i=0; i<quarterCycleUpperRight.size(); i++)
  {
     tempPoint[0] = quarterCycleUpperRight[i][0];
     tempPoint[1] = quarterCycleUpperRight[i][1];
     tempPoint[2] = 0;
     contourInImageIndexCoordinates->AddVertex( tempPoint );
  }
  // the lower right has to be parsed in reverse order
  for (int i=quarterCycleLowerRight.size()-1; i>=0; i--)
  {
     tempPoint[0] = quarterCycleLowerRight[i][0];
     tempPoint[1] = quarterCycleLowerRight[i][1];
     tempPoint[2] = 0;
     contourInImageIndexCoordinates->AddVertex( tempPoint );
  }
  for (unsigned int i=0; i<quarterCycleLowerLeft.size(); i++)
  {
     tempPoint[0] = quarterCycleLowerLeft[i][0];
     tempPoint[1] = quarterCycleLowerLeft[i][1];
     tempPoint[2] = 0;
     contourInImageIndexCoordinates->AddVertex( tempPoint );
  }
  // the upper left also has to be parsed in reverse order
  for (int i=quarterCycleUpperLeft.size()-1; i>=0; i--)
  {
     tempPoint[0] = quarterCycleUpperLeft[i][0];
     tempPoint[1] = quarterCycleUpperLeft[i][1];
     tempPoint[2] = 0;
     contourInImageIndexCoordinates->AddVertex( tempPoint );
  }

  m_MasterContour = contourInImageIndexCoordinates;

}


/**
  Just show the contour, get one point as the central point and add surrounding points to the contour.
  */
bool mitk::PaintbrushTool::OnMousePressed ( StateMachineAction*, InteractionEvent* interactionEvent )
{
  mitk::InteractionPositionEvent* positionEvent = dynamic_cast<mitk::InteractionPositionEvent*>( interactionEvent );
  //const PositionEvent* positionEvent = dynamic_cast<const PositionEvent*>(stateEvent->GetEvent());
  if (!positionEvent) return false;

  m_LastEventSender = positionEvent->GetSender();
  m_LastEventSlice = m_LastEventSender->GetSlice();

  m_MasterContour->SetClosed(true);

  return this->MouseMoved(interactionEvent, true);
}

bool mitk::PaintbrushTool::OnMouseMoved( StateMachineAction*, InteractionEvent* interactionEvent )
{
 return MouseMoved(interactionEvent, false);
}

bool mitk::PaintbrushTool::OnPrimaryButtonPressedMoved( StateMachineAction*, InteractionEvent* interactionEvent )
{
 return MouseMoved(interactionEvent, true);
}

/**
  Insert the point to the feedback contour,finish to build the contour and at the same time the painting function
  */
bool mitk::PaintbrushTool::MouseMoved(mitk::InteractionEvent* interactionEvent, bool leftMouseButtonPressed)
{
  mitk::InteractionPositionEvent* positionEvent = dynamic_cast<mitk::InteractionPositionEvent*>( interactionEvent );
  //const PositionEvent* positionEvent = dynamic_cast<const PositionEvent*>(stateEvent->GetEvent());

  CheckIfCurrentSliceHasChanged( positionEvent );

  if ( m_LastContourSize != m_Size )
  {
    UpdateContour( positionEvent );
    m_LastContourSize = m_Size;
  }

//     stateEvent->GetId() == 530
//     || stateEvent->GetId() == 534
//     || stateEvent->GetId() == 1
//     || stateEvent->GetId() == 5
//     );

  Point3D worldCoordinates = positionEvent->GetPositionInWorld();
  Point3D indexCoordinates;

  m_WorkingSlice->GetGeometry()->WorldToIndex( worldCoordinates, indexCoordinates );

  MITK_DEBUG << "Mouse at W " << worldCoordinates << std::endl;
  MITK_DEBUG << "Mouse at I " << indexCoordinates << std::endl;

  // round to nearest voxel center (abort if this hasn't changed)
  if ( m_Size % 2 == 0 ) // even
  {
    indexCoordinates[0] = ROUND( indexCoordinates[0]);// /*+ 0.5*/) + 0.5;
    indexCoordinates[1] = ROUND( indexCoordinates[1]);// /*+ 0.5*/ ) + 0.5;
  }
  else // odd
  {
    indexCoordinates[0] = ROUND( indexCoordinates[0]  ) ;
    indexCoordinates[1] = ROUND( indexCoordinates[1] ) ;
  }

  static Point3D lastPos; // uninitialized: if somebody finds out how this can be initialized in a one-liner, tell me
  if ( fabs(indexCoordinates[0] - lastPos[0]) > mitk::eps ||
       fabs(indexCoordinates[1] - lastPos[1]) > mitk::eps ||
       fabs(indexCoordinates[2] - lastPos[2]) > mitk::eps ||
       leftMouseButtonPressed
     )
  {
    lastPos = indexCoordinates;
  }
  else
  {
    MITK_DEBUG << "." << std::flush;
    return false;
  }

  MITK_DEBUG << "Mouse at C " << indexCoordinates;

  int timestep = positionEvent->GetSender()->GetTimeStep();

  ContourModel::Pointer contour = ContourModel::New();
  contour->Expand(timestep + 1);
  contour->SetClosed(true, timestep);

  ContourModel::VertexIterator it = m_MasterContour->Begin();
  ContourModel::VertexIterator end = m_MasterContour->End();

  while(it != end)
  {
    Point3D point = (*it)->Coordinates;
    point[0] += indexCoordinates[ 0 ];
    point[1] += indexCoordinates[ 1 ];

    contour->AddVertex( point, timestep );
    it++;
  }


  if (leftMouseButtonPressed)
  {
    FeedbackContourTool::FillContourInSlice( contour, timestep, m_WorkingSlice, m_PaintingPixelValue );
    m_WorkingNode->SetData(m_WorkingSlice);
    m_WorkingNode->Modified();
  }

  // visualize contour
  ContourModel::Pointer displayContour = ContourModel::New();
  displayContour->Initialize();

  //for (unsigned int index = 0; index < contour->GetNumberOfPoints(); ++index)
  //{
  //  Point3D point = contour->GetPoints()->ElementAt(index);
  //  if ( m_Size % 2 == 0 ) // even
  //  {
  //    point[0] += 0.5;
  //    point[1] += 0.5;
  //  }
  //  displayContour->AddVertex( point );
  //}

  displayContour = FeedbackContourTool::BackProjectContourFrom2DSlice( m_WorkingSlice->GetGeometry(), /*displayContour*/contour );
  SetFeedbackContour( *displayContour );

  assert( positionEvent->GetSender()->GetRenderWindow() );

  RenderingManager::GetInstance()->RequestUpdate( positionEvent->GetSender()->GetRenderWindow() );

  return true;
}


bool mitk::PaintbrushTool::OnMouseReleased( StateMachineAction*, InteractionEvent* interactionEvent )
{
  //When mouse is released write segmentationresult back into image
  mitk::InteractionPositionEvent* positionEvent = dynamic_cast<mitk::InteractionPositionEvent*>( interactionEvent );
  //const PositionEvent* positionEvent = dynamic_cast<const PositionEvent*>(stateEvent->GetEvent());
  if (!positionEvent) return false;

  this->WriteBackSegmentationResult(positionEvent, m_WorkingSlice->Clone());

  return true;
}

/**
  Called when the CTRL key is pressed. Will change the painting pixel value from 0 to 1 or from 1 to 0.
  */
bool mitk::PaintbrushTool::OnInvertLogic( StateMachineAction*, InteractionEvent* interactionEvent )
{
    // Inversion only for 0 and 1 as painting values
    if (m_PaintingPixelValue == 1)
    {
      m_PaintingPixelValue = 0;
      FeedbackContourTool::SetFeedbackContourColor( 1.0, 0.0, 0.0 );
    }
    else if (m_PaintingPixelValue == 0)
    {
      m_PaintingPixelValue = 1;
      FeedbackContourTool::SetFeedbackContourColorDefault();
    }
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    return true;
}

void mitk::PaintbrushTool::CheckIfCurrentSliceHasChanged(const InteractionPositionEvent *event)
{
    const PlaneGeometry* planeGeometry( dynamic_cast<const PlaneGeometry*> (event->GetSender()->GetCurrentWorldPlaneGeometry() ) );
    const AbstractTransformGeometry* abstractTransformGeometry( dynamic_cast<const AbstractTransformGeometry*> (event->GetSender()->GetCurrentWorldPlaneGeometry() ) );
    DataNode* workingNode( m_ToolManager->GetWorkingData(0) );

    if (!workingNode)
        return;

    Image::Pointer image = dynamic_cast<Image*>(workingNode->GetData());

    if ( !image || !planeGeometry || abstractTransformGeometry )
        return;

    if(m_CurrentPlane.IsNull() || m_WorkingSlice.IsNull())
    {
        m_CurrentPlane = const_cast<PlaneGeometry*>(planeGeometry);
        m_WorkingSlice = SegTool2D::GetAffectedImageSliceAs2DImage(event, image)->Clone();
        m_WorkingNode->ReplaceProperty( "color", workingNode->GetProperty("color") );
        m_WorkingNode->SetData(m_WorkingSlice);
    }
    else
    {
        bool isSameSlice (false);
        isSameSlice = mitk::MatrixEqualElementWise(planeGeometry->GetIndexToWorldTransform()->GetMatrix(),m_CurrentPlane->GetIndexToWorldTransform()->GetMatrix());
        isSameSlice = mitk::Equal(planeGeometry->GetIndexToWorldTransform()->GetOffset(),m_CurrentPlane->GetIndexToWorldTransform()->GetOffset());
        if (!isSameSlice)
        {
            m_ToolManager->GetDataStorage()->Remove(m_WorkingNode);
            m_CurrentPlane = NULL;
            m_WorkingSlice = NULL;
            m_WorkingNode = NULL;
            m_CurrentPlane = const_cast<PlaneGeometry*>(planeGeometry);
            m_WorkingSlice = SegTool2D::GetAffectedImageSliceAs2DImage(event, image)->Clone();

            m_WorkingNode = mitk::DataNode::New();
            m_WorkingNode->SetProperty( "levelwindow", mitk::LevelWindowProperty::New( mitk::LevelWindow(0, 1) ) );
            m_WorkingNode->SetProperty( "binary", mitk::BoolProperty::New(true) );

            m_WorkingNode->SetData(m_WorkingSlice);

            //So that the paintbrush contour vanished in the previous render window
            RenderingManager::GetInstance()->RequestUpdateAll();
        }

    }

    if(!m_ToolManager->GetDataStorage()->Exists(m_WorkingNode))
    {

        m_WorkingNode->SetProperty( "outline binary", mitk::BoolProperty::New(true) );
        m_WorkingNode->SetProperty( "color", workingNode->GetProperty("color") );
        m_WorkingNode->SetProperty( "name", mitk::StringProperty::New("Paintbrush_Node") );
        m_WorkingNode->SetProperty( "helper object", mitk::BoolProperty::New(true) );
        m_WorkingNode->SetProperty( "opacity", mitk::FloatProperty::New(0.8) );
        m_WorkingNode->SetProperty( "includeInBoundingBox", mitk::BoolProperty::New(false));
        m_WorkingNode->SetVisibility(false, mitk::BaseRenderer::GetInstance( mitk::BaseRenderer::GetRenderWindowByName("stdmulti.widget4")));

        m_ToolManager->GetDataStorage()->Add(m_WorkingNode);
    }
}

void mitk::PaintbrushTool::OnToolManagerWorkingDataModified()
{
  //Here we simply set the current working slice to null. The next time the mouse is moved
  //within a renderwindow a new slice will be extracted from the new working data
  m_WorkingSlice = 0;
}
