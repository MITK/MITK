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
  : FeedbackContourTool("PressMoveReleaseWithCTRLInversionAllMouseMoves"),
    m_PaintingPixelValue(paintingPixelValue),
    m_LastContourSize(0), // other than initial mitk::PaintbrushTool::m_Size (around l. 28)
    m_WorkingImage(nullptr),
    m_ImageObserverTag(0),
    m_LastTimeStep(0)
{
  m_MasterContour = ContourModel::New();
  m_MasterContour->Initialize();
  m_CurrentPlane = nullptr;

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

  std::vector<mitk::BaseRenderer*> windows = {
    mitk::BaseRenderer::GetInstance(mitk::BaseRenderer::GetRenderWindowByName("stdmulti.widget1")),
    mitk::BaseRenderer::GetInstance(mitk::BaseRenderer::GetRenderWindowByName("stdmulti.widget2")),
    mitk::BaseRenderer::GetInstance(mitk::BaseRenderer::GetRenderWindowByName("stdmulti.widget3"))
  };

  for (auto window : windows) {
    window->GetSliceNavigationController()->ConnectGeometrySliceEvent(this, false);
  }
}

void mitk::PaintbrushTool::Deactivated()
{
  std::vector<mitk::BaseRenderer*> windows = {
    mitk::BaseRenderer::GetInstance(mitk::BaseRenderer::GetRenderWindowByName("stdmulti.widget1")),
    mitk::BaseRenderer::GetInstance(mitk::BaseRenderer::GetRenderWindowByName("stdmulti.widget2")),
    mitk::BaseRenderer::GetInstance(mitk::BaseRenderer::GetRenderWindowByName("stdmulti.widget3"))
  };

  for (auto window : windows) {
    window->GetSliceNavigationController()->Disconnect(this);
  }

  FeedbackContourTool::SetFeedbackContourVisible(false);
  if (m_ToolManager->GetDataStorage()->Exists(m_WorkingNode))
      m_ToolManager->GetDataStorage()->Remove(m_WorkingNode);
  m_WorkingSlice = nullptr;
  m_CurrentPlane = nullptr;

  if (m_WorkingImage && m_ImageObserverTag)
  {
    m_WorkingImage->RemoveObserver(m_ImageObserverTag);
  }
  m_WorkingImage = nullptr;

  m_ToolManager->WorkingDataChanged -= mitk::MessageDelegate<mitk::PaintbrushTool>( this, &mitk::PaintbrushTool::OnToolManagerWorkingDataModified );

  Superclass::Deactivated();
}

void mitk::PaintbrushTool::SetGeometrySlice(const itk::EventObject& geometrySliceEvent)
{
  const SliceNavigationController::GeometrySliceEvent* sliceEvent =
    dynamic_cast<const SliceNavigationController::GeometrySliceEvent *>(&geometrySliceEvent);
  int pos = sliceEvent->GetPos();
  const mitk::TimeGeometry* timeGeom = sliceEvent->GetTimeGeometry();
  const mitk::BaseGeometry* baseGeometry = timeGeom->GetGeometryForTimeStep(m_LastTimeStep);
  const mitk::SlicedGeometry3D *slicedGeometry = dynamic_cast<const mitk::SlicedGeometry3D*>(baseGeometry);
  const mitk::PlaneGeometry* planeGeometry = slicedGeometry->GetPlaneGeometry(pos);

  MouseMovedImpl(planeGeometry, false);
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


void mitk::PaintbrushTool::UpdateContour()
{
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
void mitk::PaintbrushTool::OnMousePressed ( StateMachineAction*, InteractionEvent* interactionEvent )
{
  if (m_WorkingSlice.IsNull()) return;

  mitk::InteractionPositionEvent* positionEvent = dynamic_cast<mitk::InteractionPositionEvent*>( interactionEvent );
  m_WorkingSlice->GetGeometry()->WorldToIndex( positionEvent->GetPlanePositionInWorld(), m_LastPosition );

  if (!positionEvent) return;

  m_LastEventSender = positionEvent->GetSender();
  m_LastEventSlice = m_LastEventSender->GetSlice();

  m_MasterContour->SetClosed(true);
  this->MouseMoved(interactionEvent, true);
}

void mitk::PaintbrushTool::OnMouseMoved( StateMachineAction*, InteractionEvent* interactionEvent )
{
 MouseMoved(interactionEvent, false);
}

void mitk::PaintbrushTool::OnPrimaryButtonPressedMoved( StateMachineAction*, InteractionEvent* interactionEvent )
{
 MouseMoved(interactionEvent, true);
}

/**
  Insert the point to the feedback contour,finish to build the contour and at the same time the painting function
  */
void mitk::PaintbrushTool::MouseMoved(mitk::InteractionEvent* interactionEvent, bool leftMouseButtonPressed)
{
  mitk::InteractionPositionEvent* positionEvent = dynamic_cast<mitk::InteractionPositionEvent*>( interactionEvent );

  m_LastTimeStep = positionEvent->GetSender()->GetTimeStep();
  m_LastWorldCoordinates = positionEvent->GetPlanePositionInWorld();

  MouseMovedImpl(positionEvent->GetSender()->GetCurrentWorldPlaneGeometry(), leftMouseButtonPressed);

  assert( positionEvent->GetSender()->GetRenderWindow() );

  RenderingManager::GetInstance()->RequestUpdate( positionEvent->GetSender()->GetRenderWindow() );
}

void mitk::PaintbrushTool::MouseMovedImpl(const mitk::PlaneGeometry* planeGeometry, bool leftMouseButtonPressed)
{
  CheckIfCurrentSliceHasChanged(planeGeometry);

  if (m_LastContourSize != m_Size) {
    UpdateContour();
    m_LastContourSize = m_Size;
  }

  Point3D indexCoordinates;
  m_WorkingSlice->GetGeometry()->WorldToIndex(m_LastWorldCoordinates, indexCoordinates);

  MITK_DEBUG << "Mouse at W " << m_LastWorldCoordinates << std::endl;
  MITK_DEBUG << "Mouse at I " << m_LastWorldCoordinates << std::endl;

  // round to nearest voxel center (abort if this hasn't changed)
  if (m_Size % 2 == 0) // even
  {
    indexCoordinates[0] = ROUND(indexCoordinates[0]);// /*+ 0.5*/) + 0.5;
    indexCoordinates[1] = ROUND(indexCoordinates[1]);// /*+ 0.5*/ ) + 0.5;
  } else // odd
  {
    indexCoordinates[0] = ROUND(indexCoordinates[0]);
    indexCoordinates[1] = ROUND(indexCoordinates[1]);
  }

  static Point3D lastPos; // uninitialized: if somebody finds out how this can be initialized in a one-liner, tell me
  if (fabs(indexCoordinates[0] - lastPos[0]) > mitk::eps ||
    fabs(indexCoordinates[1] - lastPos[1]) > mitk::eps ||
    fabs(indexCoordinates[2] - lastPos[2]) > mitk::eps ||
    leftMouseButtonPressed
    )
  {
    lastPos = indexCoordinates;
  } else
  {
    MITK_DEBUG << "." << std::flush;
    return;
  }

  MITK_DEBUG << "Mouse at C " << indexCoordinates;

  ContourModel::Pointer contour = ContourModel::New();
  contour->SetClosed(true);

  ContourModel::VertexIterator it = m_MasterContour->Begin();
  ContourModel::VertexIterator end = m_MasterContour->End();

  Point3D roundedWorldPoint;
  m_WorkingSlice->GetGeometry()->IndexToWorld(indexCoordinates, roundedWorldPoint);

  while (it != end)
  {
    Point3D point = (*it)->Coordinates;

    Point3D worldPoint = roundedWorldPoint;

    offsetPointForContour(worldPoint, point);

    m_WorkingSlice->GetGeometry()->WorldToIndex(worldPoint, point);

    contour->AddVertex(point);
    it++;
  }

  if (leftMouseButtonPressed)
  {
    const double dist = indexCoordinates.EuclideanDistanceTo(m_LastPosition);
    const double radius = static_cast<double>(m_Size) / 2.0;

    // draw it the old way
    FeedbackContourTool::FillContourInSlice(contour, m_WorkingSlice, m_PaintingPixelValue);
    m_WorkingNode->SetData(m_WorkingSlice);
    m_WorkingNode->Modified();

    // if points are >= radius away draw rectangle to fill empty holes
    // in between the 2 points
    if (dist > radius)
    {
      const mitk::Point3D& currentPos = indexCoordinates;
      mitk::Point3D direction;
      mitk::Point3D vertex;
      mitk::Point3D normalOffset;

      direction[0] = indexCoordinates[0] - m_LastPosition[0];
      direction[1] = indexCoordinates[1] - m_LastPosition[1];
      direction[2] = indexCoordinates[2] - m_LastPosition[2];

      direction[0] = direction.GetVnlVector().normalize()[0];
      direction[1] = direction.GetVnlVector().normalize()[1];
      direction[2] = direction.GetVnlVector().normalize()[2];

      // 90 degrees rotation of direction
      normalOffset[0] = -1.0 * direction[1] * radius;
      normalOffset[1] = direction[0] * radius;

      contour->Clear();

      Point3D worldLastPosition;
      m_WorkingSlice->GetGeometry()->IndexToWorld(m_LastPosition, worldLastPosition);

      Point3D worldCurrentPosition;
      m_WorkingSlice->GetGeometry()->IndexToWorld(currentPos, worldCurrentPosition);

      Point3D worldVertex;

      // Upper left corner
      worldVertex = worldLastPosition;
      offsetPointForContour(worldVertex, normalOffset);
      m_WorkingSlice->GetGeometry()->WorldToIndex(worldVertex, vertex);

      contour->AddVertex(vertex);

      // Upper right corner
      worldVertex = worldCurrentPosition;
      offsetPointForContour(worldVertex, normalOffset);
      m_WorkingSlice->GetGeometry()->WorldToIndex(worldVertex, vertex);

      contour->AddVertex(vertex);

      normalOffset[0] = -normalOffset[0];
      normalOffset[1] = -normalOffset[1];

      // Lower right corner
      worldVertex = worldCurrentPosition;
      offsetPointForContour(worldVertex, normalOffset);
      m_WorkingSlice->GetGeometry()->WorldToIndex(worldVertex, vertex);

      contour->AddVertex(vertex);

      // Lower left corner
      worldVertex = worldLastPosition;
      offsetPointForContour(worldVertex, normalOffset);
      m_WorkingSlice->GetGeometry()->WorldToIndex(worldVertex, vertex);

      contour->AddVertex(vertex);

      FeedbackContourTool::FillContourInSlice(contour, m_WorkingSlice, m_PaintingPixelValue);
      m_WorkingNode->SetData(m_WorkingSlice);
      m_WorkingNode->Modified();
    }
  }

  m_LastPosition = indexCoordinates;

  // visualize contour
  ContourModel::Pointer displayContour = this->GetFeedbackContour();
  displayContour->Clear();

  ContourModel::Pointer tmp = FeedbackContourTool::BackProjectContourFrom2DSlice(m_WorkingSlice->GetGeometry(), /*displayContour*/contour);

  // copy transformed contour into display contour
  it = tmp->Begin();
  end = tmp->End();

  while (it != end)
  {
    Point3D point = (*it)->Coordinates;

    displayContour->AddVertex(point);
    it++;
  }

  FeedbackContourTool::SetFeedbackContour(m_LastTimeStep == 0 ? displayContour : ContourModelUtils::MoveZerothContourTimeStep(displayContour, m_LastTimeStep));
  m_FeedbackContourNode->GetData()->Modified();
}

void mitk::PaintbrushTool::OnMouseReleased( StateMachineAction*, InteractionEvent* interactionEvent )
{
  //When mouse is released write segmentationresult back into image
  mitk::InteractionPositionEvent* positionEvent = dynamic_cast<mitk::InteractionPositionEvent*>( interactionEvent );
  if (!positionEvent) return;

  CheckIfCurrentSliceHasChanged(positionEvent->GetSender()->GetCurrentWorldPlaneGeometry());
  this->WriteBackSegmentationResult(positionEvent, m_WorkingSlice->Clone());
}

/**
  Called when the CTRL key is pressed. Will change the painting pixel value from 0 to 1 or from 1 to 0.
  */
void mitk::PaintbrushTool::OnInvertLogic( StateMachineAction*, InteractionEvent* interactionEvent )
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
}

void mitk::PaintbrushTool::CheckIfCurrentSliceHasChanged(const mitk::PlaneGeometry* planeGeometry)
{
    const AbstractTransformGeometry* abstractTransformGeometry( dynamic_cast<const AbstractTransformGeometry*> (planeGeometry) );
    DataNode* workingNode( m_ToolManager->GetWorkingData(0) );

    if (!workingNode)
    {
      return;
    }

    Image::Pointer image = dynamic_cast<Image*>(workingNode->GetData());

    if (!image || !planeGeometry || abstractTransformGeometry)
    {
      return;
    }

    if (m_WorkingImage != image)
    {
      // observe Modified() event of image
      if (m_WorkingImage && m_ImageObserverTag)
      {
        m_WorkingImage->RemoveObserver(m_ImageObserverTag);
      }

      m_WorkingImage = image;

      itk::ReceptorMemberCommand<PaintbrushTool>::Pointer command = itk::ReceptorMemberCommand<PaintbrushTool>::New();
      command->SetCallbackFunction(this, &PaintbrushTool::OnToolManagerImageModified);
      m_ImageObserverTag = m_WorkingImage->AddObserver(itk::ModifiedEvent(), command);
    }

    if (m_CurrentPlane.IsNull() || m_WorkingSlice.IsNull())
    {
        m_CurrentPlane = const_cast<PlaneGeometry*>(planeGeometry);
        m_WorkingSlice = SegTool2D::GetAffectedImageSliceAs2DImage(planeGeometry, image, m_LastTimeStep)->Clone();
        m_WorkingNode->ReplaceProperty( "color", workingNode->GetProperty("color") );
        m_WorkingNode->SetData(m_WorkingSlice);
        resampleDecomposition(m_WorkingImage, m_WorkingSlice);
    }
    else
    {
        bool isSameSlice (false);
        isSameSlice = mitk::MatrixEqualElementWise(planeGeometry->GetIndexToWorldTransform()->GetMatrix(),m_CurrentPlane->GetIndexToWorldTransform()->GetMatrix());
        isSameSlice = mitk::Equal(planeGeometry->GetIndexToWorldTransform()->GetOffset(),m_CurrentPlane->GetIndexToWorldTransform()->GetOffset());
        if (!isSameSlice)
        {
            m_ToolManager->GetDataStorage()->Remove(m_WorkingNode);
            m_CurrentPlane = nullptr;
            m_WorkingSlice = nullptr;
            m_WorkingNode = nullptr;
            m_CurrentPlane = const_cast<PlaneGeometry*>(planeGeometry);
            m_WorkingSlice = SegTool2D::GetAffectedImageSliceAs2DImage(planeGeometry, image, m_LastTimeStep)->Clone();
            resampleDecomposition(m_WorkingImage, m_WorkingSlice);

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
  m_WorkingSlice = nullptr;
}

void mitk::PaintbrushTool::OnToolManagerImageModified(const itk::EventObject&)
{
  OnToolManagerWorkingDataModified();
}
