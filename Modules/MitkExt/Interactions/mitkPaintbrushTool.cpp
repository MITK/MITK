/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision: 1.12 $
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

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
}

void mitk::PaintbrushTool::SetSize(int value)
{
  m_Size = value;
}

void mitk::PaintbrushTool::UpdateContour(const StateEvent* stateEvent)
{
    //MITK_INFO<<"Update...";
  // examine stateEvent and create a contour that matches the pixel mask that we are going to draw
  const PositionEvent* positionEvent = dynamic_cast<const PositionEvent*>(stateEvent->GetEvent());
  if (!positionEvent) return;
  
  // create a copy of this slice (at least match the pixel sizes/spacings),
  // then draw the desired mask on it and create a contour from it

  // Convert to ipMITKSegmentationTYPE (because getting pixels relys on that data type)
  itk::Image< ipMITKSegmentationTYPE, 2 >::Pointer correctPixelTypeImage;
  CastToItkImage(m_WorkingSlice/*dynamic_cast<mitk::Image*>(m_WorkingNode->GetData())*/, correctPixelTypeImage );
  assert (correctPixelTypeImage.IsNotNull() );

  itk::Image< ipMITKSegmentationTYPE, 2 >::DirectionType imageDirection;
  imageDirection.SetIdentity();
  correctPixelTypeImage->SetDirection(imageDirection);

  Image::Pointer temporarySlice = Image::New();
  CastToMitkImage( correctPixelTypeImage, temporarySlice );

  //mitkIpPicDescriptor* stupidClone = mitkIpPicClone( temporarySlice->GetSliceData()->GetPicDescriptor() );
  mitkIpPicDescriptor* stupidClone = mitkIpPicNew();
  CastToIpPicDescriptor( temporarySlice->GetSliceData(), stupidClone );
  unsigned int pixelWidth  = m_Size + 1;
  unsigned int pixelHeight = m_Size + 1;

  if ( stupidClone->n[0] <= pixelWidth || stupidClone->n[1] <= pixelHeight )
  {
    MITK_INFO << "Brush size is bigger than your working image. Reconsider this...\n"
                "(Or tell your progammer until (s)he fixes this message.)" << std::endl;
    mitkIpPicFree( stupidClone );
    return;
  }
  
  unsigned int lineLength( stupidClone->n[0] );
  unsigned int oneContourOffset(0);
  float circleCenterX = (float)m_Size / 2.0;
  float circleCenterY = (float)m_Size / 2.0;
  for (unsigned int x = 0; x <= pixelWidth; ++x)
  {
    for (unsigned int y = 0; y <= pixelHeight; ++y)
    {
      unsigned int offset = lineLength * y + x;
      ipMITKSegmentationTYPE* current = (ipMITKSegmentationTYPE*)stupidClone->data + offset;

      float pixelCenterX = x + 0.5;
      float pixelCenterY = y + 0.5;

      float xoff = pixelCenterX - circleCenterX;
      float yoff = pixelCenterY - circleCenterY;

      bool inside = xoff * xoff + yoff * yoff < (m_Size * m_Size) / 4.0; // no idea, if this would work for ellipses
      if (inside)
      {
        *current = 1;
        oneContourOffset = offset;
      }
      else
      {
        *current = 0;
      }
    }
  }
      
  int numberOfContourPoints( 0 );
  int newBufferSize( 0 );
  float* contourPoints = ipMITKSegmentationGetContour8N( stupidClone, oneContourOffset, numberOfContourPoints, newBufferSize ); // memory allocated with malloc
  if (!contourPoints) 
  {
    mitkIpPicFree( stupidClone );
    return;
  }

  // copy point from float* to mitk::Contour 
  Contour::Pointer contourInImageIndexCoordinates = Contour::New();
  contourInImageIndexCoordinates->Initialize();
  Point3D newPoint;
  //ipMITKSegmentationGetContour8N returns all points, which causes vtk warnings, since the first and the last points are coincident.
  //leaving the last point out, the contour is still drawn correctly
  for (int index = 0; index < numberOfContourPoints-1; ++index)
  {
    newPoint[0] = contourPoints[ 2 * index + 0 ] - circleCenterX; // master contour should be centered around (0,0)
    newPoint[1] = contourPoints[ 2 * index + 1] - circleCenterY;
    newPoint[2] = 0.0;
    MITK_DEBUG << "Point [" << index << "] (" << newPoint[0] << ", " << newPoint[1] << ")" << std::endl;

    contourInImageIndexCoordinates->AddVertex( newPoint );
  }

  free(contourPoints);

  m_MasterContour = contourInImageIndexCoordinates;

  // The PicDescriptor is only REFERENCING(!) the data, the temporarySlice takes care of deleting the data also the descriptor is pointing on
  // because they got allocated by the ImageDataItem, not the descriptor.
  stupidClone->data = NULL;
  mitkIpPicFree( stupidClone );
}


/**
  Just show the contour, get one point as the central point and add surrounding points to the contour.
  */
bool mitk::PaintbrushTool::OnMousePressed (Action* action, const StateEvent* stateEvent)
{
  return this->OnMouseMoved(action, stateEvent);
}


/**
  Insert the point to the feedback contour,finish to build the contour and at the same time the painting function
  */
bool mitk::PaintbrushTool::OnMouseMoved   (Action* itkNotUsed(action), const StateEvent* stateEvent)
{
    const PositionEvent* positionEvent = dynamic_cast<const PositionEvent*>(stateEvent->GetEvent());
    if (!positionEvent) return false;

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
  static bool lastLeftMouseButtonPressed(false);
  if ( fabs(indexCoordinates[0] - lastPos[0]) > mitk::eps ||
       fabs(indexCoordinates[1] - lastPos[1]) > mitk::eps ||
       fabs(indexCoordinates[2] - lastPos[2]) > mitk::eps ||
       leftMouseButtonPressed != lastLeftMouseButtonPressed
     )
  {
    lastPos = indexCoordinates;
    lastLeftMouseButtonPressed = leftMouseButtonPressed;
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

    MITK_DEBUG << "Contour point [" << index << "] :" << point;
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
    this->WriteBackSegmentationResult(positionEvent, m_WorkingSlice);
//  FeedbackContourTool::SetFeedbackContourVisible(false);

  return true;
}

/**
  Called when the CTRL key is pressed. Will change the painting pixel value from 0 to 1 or from 1 to 0. 
  */
bool mitk::PaintbrushTool::OnInvertLogic(Action* action, const StateEvent* stateEvent)
{
  if (!FeedbackContourTool::OnInvertLogic(action, stateEvent)) return false;

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
        }

    }

    if(!m_ToolManager->GetDataStorage()->Exists(m_WorkingNode))
    {

        m_WorkingNode->SetProperty( "outline binary", mitk::BoolProperty::New(true) );
        m_WorkingNode->SetProperty( "color", workingNode->GetProperty("color") );
        m_WorkingNode->SetProperty( "texture interpolation", mitk::BoolProperty::New(true) );
        m_WorkingNode->SetProperty( "layer", mitk::IntProperty::New( 100 ) );
        m_WorkingNode->SetProperty( "name", mitk::StringProperty::New("Paintbrush_Node") );
        m_WorkingNode->SetProperty( "helper object", mitk::BoolProperty::New(true) );
        m_WorkingNode->SetProperty( "opacity", mitk::FloatProperty::New(0.8) );
        m_WorkingNode->SetProperty( "includeInBoundingBox", mitk::BoolProperty::New(false));

        m_ToolManager->GetDataStorage()->Add(m_WorkingNode);
    }
}
