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

#include "mitkLevelWindowProperty.h"

#define ROUND(a)     ((a)>0 ? (int)((a)+0.5) : -(int)(0.5-(a)))

int mitk::PaintbrushTool::m_Size = 1;

mitk::PaintbrushTool::PaintbrushTool(int paintingPixelValue)
:FeedbackContourTool("PressMoveReleaseWithCTRLInversionAllMouseMoves"),
 m_PaintingPixelValue(paintingPixelValue),
 m_LastContourSize(0) // other than initial mitk::PaintbrushTool::m_Size (around l. 28)
{
  // great magic numbers
  CONNECT_ACTION( 80, OnMousePressed );
  CONNECT_ACTION( 90, OnMouseMoved );
  CONNECT_ACTION( 42, OnMouseReleased );
  CONNECT_ACTION( 49014, OnInvertLogic );


  m_MasterContour = Contour::New();
  m_MasterContour->Initialize();
  m_CurrentPlane = NULL;

  m_WorkingNode = DataNode::New();
  m_WorkingNode->SetProperty( "levelwindow", mitk::LevelWindowProperty::New( mitk::LevelWindow(0, 1) ) );
  m_WorkingNode->SetProperty( "binary", mitk::BoolProperty::New(true) );
}

mitk::PaintbrushTool::~PaintbrushTool()
{
}

void mitk::PaintbrushTool::Activated()
{
  Superclass::Activated();
  FeedbackContourTool::SetFeedbackContourVisible(true);
  SizeChanged.Send(m_Size);
}

void mitk::PaintbrushTool::Deactivated()
{
  FeedbackContourTool::SetFeedbackContourVisible(false);
  if (m_ToolManager->GetDataStorage()->Exists(m_WorkingNode))
      m_ToolManager->GetDataStorage()->Remove(m_WorkingNode);
  Superclass::Deactivated();
  m_WorkingSlice = NULL;
  m_CurrentPlane = NULL;
}

void mitk::PaintbrushTool::SetSize(int value)
{
  m_Size = value;
}

mitk::Point2D mitk::PaintbrushTool::upperLeft(mitk::Point2D p)
{
   p[0] -= 0.5;
   p[1] -= 0.5;
   return p;
}


void mitk::PaintbrushTool::UpdateContour(const StateEvent* stateEvent)
{
  //MITK_INFO<<"Update...";
  // examine stateEvent and create a contour that matches the pixel mask that we are going to draw
  const PositionEvent* positionEvent = dynamic_cast<const PositionEvent*>(stateEvent->GetEvent());
  if (!positionEvent) return;

  // Get Spacing of current Slice
  //mitk::Vector3D vSpacing = m_WorkingSlice->GetSlicedGeometry()->GetGeometry2D(0)->GetSpacing();

  //
  // Draw a contour in Square according to selected brush size
  //
  int radius = (m_Size+1)/2;
  Contour::Pointer contourInImageIndexCoordinates = Contour::New();
  contourInImageIndexCoordinates->Initialize();


  // we will compute the control points for the upper left quarter part of a circle contour
  // and determine the remaining points by mirroring
  std::vector< mitk::Point2D > fullCycle;
  std::vector< mitk::Point2D > quarterCycleUpperRight;
  std::vector< mitk::Point2D > quarterCycleLowerRight;
  std::vector< mitk::Point2D > quarterCycleLowerLeft;
  std::vector< mitk::Point2D > quarterCycleUpperLeft;


  mitk::Point2D curPoint;
  bool curPointIsInside = true;
  curPoint[0] = 0;
  curPoint[1] = radius;
  quarterCycleUpperRight.push_back(curPoint);

  while (curPoint[1] >= 0)
  {
     // Move right until pixel is outside circle
     while( curPointIsInside )
     {
        // increment posX and chec
        curPoint[0]++;
        float len = sqrt(curPoint[0] * curPoint[0] + curPoint[1] + curPoint[1]);
        if ( len > radius )
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
        float len = sqrt(curPoint[0] * curPoint[0] + curPoint[1] + curPoint[1]);
        if ( len <= radius )
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
  std::vector< mitk::Point2D >::const_iterator it = quarterCycleUpperRight.begin();
  while( it != quarterCycleUpperRight.end() )
  {
     mitk::Point2D p;
     p = *it;
     p[1] *= -1;
     quarterCycleLowerRight.push_back(p);

     p[0] *= -1;
     quarterCycleLowerLeft.push_back(p);

     p[1] *= -1;
     quarterCycleUpperLeft.push_back(p);

     it++;
  }

  // now fill contour
  mitk::Point3D tempPoint;
  for (int i=0; i<quarterCycleUpperRight.size(); i++)
  {
     tempPoint[0] = quarterCycleUpperRight[i][0];
     tempPoint[1] = quarterCycleUpperRight[i][1];
     tempPoint[2] = 0;
     contourInImageIndexCoordinates->AddVertex( tempPoint  );
  }
  for (int i=quarterCycleUpperRight.size()-1; i>=0; i++)
  {
     tempPoint[0] = quarterCycleLowerRight[i][0];
     tempPoint[1] = quarterCycleLowerRight[i][1];
     tempPoint[2] = 0;
     contourInImageIndexCoordinates->AddVertex( tempPoint);
  }
  for (int i=0; i<quarterCycleLowerLeft.size(); i++)
  {
     tempPoint[0] = quarterCycleLowerLeft[i][0];
     tempPoint[1] = quarterCycleLowerLeft[i][1];
     tempPoint[2] = 0;
     contourInImageIndexCoordinates->AddVertex( tempPoint );
  }
  for (int i=quarterCycleUpperLeft.size()-1; i>=0; i++)
  {
     tempPoint[0] = quarterCycleUpperLeft[i][0];
     tempPoint[1] = quarterCycleUpperLeft[i][1];
     tempPoint[2] = 0;
     contourInImageIndexCoordinates->AddVertex( tempPoint );
  }


/*
  mitk::Point3D pIndex;
  pIndex[0] = 0;
  pIndex[1] = 0;
  pIndex[2] = 0;

  mitk::Point3D p0,p1,p2,p3;
  p0 = p1 = p2 = p3 = pIndex;

  p0[0] -=0.5 * m_Size;
  p0[1] -=0.5 * m_Size;
  p1[0] -=0.5 * m_Size;
  p1[1] +=0.5 * m_Size;
  p2[0] +=0.5 * m_Size;
  p2[1] +=0.5 * m_Size;
  p3[0] +=0.5 * m_Size;
  p3[1] -=0.5 * m_Size;

  contourInImageIndexCoordinates->AddVertex( p0 );
  contourInImageIndexCoordinates->AddVertex( p1 );
  contourInImageIndexCoordinates->AddVertex( p2 );
  contourInImageIndexCoordinates->AddVertex( p3 );
*/


  m_MasterContour = contourInImageIndexCoordinates;

}


/**
  Just show the contour, get one point as the central point and add surrounding points to the contour.
  */
bool mitk::PaintbrushTool::OnMousePressed (Action* action, const StateEvent* stateEvent)
{
  const PositionEvent* positionEvent = dynamic_cast<const PositionEvent*>(stateEvent->GetEvent());
  if (!positionEvent) return false;

  m_LastEventSender = positionEvent->GetSender();
  m_LastEventSlice = m_LastEventSender->GetSlice();

  return this->OnMouseMoved(action, stateEvent);
}


/**
  Insert the point to the feedback contour,finish to build the contour and at the same time the painting function
  */
bool mitk::PaintbrushTool::OnMouseMoved   (Action* itkNotUsed(action), const StateEvent* stateEvent)
{
  const PositionEvent* positionEvent = dynamic_cast<const PositionEvent*>(stateEvent->GetEvent());

  CheckIfCurrentSliceHasChanged(positionEvent);

  if ( m_LastContourSize != m_Size )
  {
    UpdateContour( stateEvent );
    m_LastContourSize = m_Size;
  }

  bool leftMouseButtonPressed(
    stateEvent->GetId() == 530
    || stateEvent->GetId() == 534
    || stateEvent->GetId() == 1
    || stateEvent->GetId() == 5
    );

  Point3D worldCoordinates = positionEvent->GetWorldPosition();
  Point3D indexCoordinates;

  m_WorkingSlice->GetGeometry()->WorldToIndex( worldCoordinates, indexCoordinates );

  MITK_DEBUG << "Mouse at W " << worldCoordinates << std::endl;
  MITK_DEBUG << "Mouse at I " << indexCoordinates << std::endl;

  // round to nearest voxel center (abort if this hasn't changed)
  if ( m_Size % 2 == 0 ) // even
  {
    indexCoordinates[0] = ROUND( indexCoordinates[0] /*+ 0.5*/) + 0.5;
    indexCoordinates[1] = ROUND( indexCoordinates[1] /*+ 0.5*/ ) + 0.5;
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

  Contour::Pointer contour = Contour::New();
  contour->Initialize();

  for (unsigned int index = 0; index < m_MasterContour->GetNumberOfPoints(); ++index)
  {
    Point3D point = m_MasterContour->GetPoints()->ElementAt(index);
    point[0] += indexCoordinates[ 0 ];
    point[1] += indexCoordinates[ 1 ];

    contour->AddVertex( point );
  }


  if (leftMouseButtonPressed)
  {
    FeedbackContourTool::FillContourInSlice( contour, m_WorkingSlice, m_PaintingPixelValue );
    m_WorkingNode->SetData(m_WorkingSlice);
    m_WorkingNode->Modified();
  }

  // visualize contour
  Contour::Pointer displayContour = Contour::New();
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


bool mitk::PaintbrushTool::OnMouseReleased(Action* /*action*/, const StateEvent* stateEvent)
{
    //When mouse is released write segmentationresult back into image
    const PositionEvent* positionEvent = dynamic_cast<const PositionEvent*>(stateEvent->GetEvent());
    if (!positionEvent) return false;
    this->WriteBackSegmentationResult(positionEvent, m_WorkingSlice->Clone());

  return true;
}

/**
  Called when the CTRL key is pressed. Will change the painting pixel value from 0 to 1 or from 1 to 0.
  */
bool mitk::PaintbrushTool::OnInvertLogic(Action* itkNotUsed(action), const StateEvent* stateEvent)
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

void mitk::PaintbrushTool::CheckIfCurrentSliceHasChanged(const PositionEvent *event)
{
    const PlaneGeometry* planeGeometry( dynamic_cast<const PlaneGeometry*> (event->GetSender()->GetCurrentWorldGeometry2D() ) );
    DataNode* workingNode( m_ToolManager->GetWorkingData(0) );

    if (!workingNode)
        return;

    Image::Pointer image = dynamic_cast<Image*>(workingNode->GetData());

    if ( !image || !planeGeometry )
        return;

    if(m_CurrentPlane.IsNull())
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
