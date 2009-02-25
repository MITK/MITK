/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile: mitkPropertyManager.cpp,v $
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
#include "ipSegmentation.h"

#define ROUND(a)     ((a)>0 ? (int)((a)+0.5) : -(int)(0.5-(a)))

int mitk::PaintbrushTool::m_Size = 1;

mitk::PaintbrushTool::PaintbrushTool(int paintingPixelValue)
:FeedbackContourTool("PressMoveReleaseWithCTRLInversionAllMouseMoves"),
 m_PaintingPixelValue(paintingPixelValue),
 m_LastContourSize(0) // other than initial mitk::PaintbrushTool::m_Size (around l. 28)
{
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
  Superclass::Deactivated();
}

void mitk::PaintbrushTool::SetSize(int value)
{
  m_Size = value;
}

void mitk::PaintbrushTool::UpdateContour(const StateEvent* stateEvent)
{
  // examine stateEvent and create a contour that matches the pixel mask that we are going to draw
  const PositionEvent* positionEvent = dynamic_cast<const PositionEvent*>(stateEvent->GetEvent());
  if (!positionEvent) return;
  
  Image::Pointer m_WorkingSlice = FeedbackContourTool::GetAffectedWorkingSlice( positionEvent );
  if (m_WorkingSlice.IsNull()) return;
  // create a copy of this slice (at least match the pixel sizes/spacings),
  // then draw the desired mask on it and create a contour from it

  // Convert to ipMITKSegmentationTYPE (because getting pixels relys on that data type)
  itk::Image< ipMITKSegmentationTYPE, 2 >::Pointer correctPixelTypeImage;
  CastToItkImage( m_WorkingSlice, correctPixelTypeImage );
  assert (correctPixelTypeImage.IsNotNull() );

  itk::Image< ipMITKSegmentationTYPE, 2 >::DirectionType imageDirection;
  imageDirection.SetIdentity();
  correctPixelTypeImage->SetDirection(imageDirection);

  Image::Pointer temporarySlice = Image::New();
  CastToMitkImage( correctPixelTypeImage, temporarySlice );

  ipPicDescriptor* stupidClone = ipPicClone( temporarySlice->GetSliceData()->GetPicDescriptor() );
  unsigned int pixelWidth  = m_Size + 1;
  unsigned int pixelHeight = m_Size + 1;

  if ( stupidClone->n[0] <= pixelWidth || stupidClone->n[1] <= pixelHeight )
  {
    std::cout << "Brush size is bigger than your working image. Reconsider this...\n"
                 "(Or tell your progammer until (s)he fixes this message.)" << std::endl;
    ipPicFree( stupidClone );
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
    ipPicFree( stupidClone );
    return;
  }

  // copy point from float* to mitk::Contour 
  Contour::Pointer contourInImageIndexCoordinates = Contour::New();
  contourInImageIndexCoordinates->Initialize();
  Point3D newPoint;
  for (int index = 0; index < numberOfContourPoints; ++index)
  {
    newPoint[0] = contourPoints[ 2 * index + 0 ] - circleCenterX; // master contour should be centered around (0,0)
    newPoint[1] = contourPoints[ 2 * index + 1] - circleCenterY;
    newPoint[2] = 0;
    //std::cout << "(" << newPoint[0] << ", " << newPoint[1] << ")" << std::endl;

    contourInImageIndexCoordinates->AddVertex( newPoint );
  }

  free(contourPoints);

  m_MasterContour = contourInImageIndexCoordinates;

  ipPicFree( stupidClone );
}


/**
  Just show the contour, get one point as the central point and add surrounding points to the contour.
  */
bool mitk::PaintbrushTool::OnMousePressed (Action* action, const StateEvent* stateEvent)
{
  if (FeedbackContourTool::OnMousePressed( action, stateEvent ))
  {
    const PositionEvent* positionEvent = dynamic_cast<const PositionEvent*>(stateEvent->GetEvent());
    if (positionEvent)
    {
      UpdateContour( stateEvent );
    }
  }
  
  return this->OnMouseMoved(action, stateEvent);
  /*

  return true;
  */
}


/**
  Insert the point to the feedback contour,finish to build the contour and at the same time the painting function
  */
bool mitk::PaintbrushTool::OnMouseMoved   (Action* itkNotUsed(action), const StateEvent* stateEvent)
{
  bool leftMouseButtonPressed(
          stateEvent->GetId() == 530
       || stateEvent->GetId() == 1
       || stateEvent->GetId() == 5
                             );

  const PositionEvent* positionEvent = dynamic_cast<const PositionEvent*>(stateEvent->GetEvent());
  if (!positionEvent) return false;

  if ( m_LastContourSize != m_Size )
  {
    UpdateContour( stateEvent );
    m_LastContourSize = m_Size;
  }

  DataTreeNode* workingNode( m_ToolManager->GetWorkingData(0) );
  if (!workingNode) return false;

  Image* image = dynamic_cast<Image*>(workingNode->GetData());
  const PlaneGeometry* planeGeometry( dynamic_cast<const PlaneGeometry*> (positionEvent->GetSender()->GetCurrentWorldGeometry2D() ) );
  if ( !image || !planeGeometry ) return false;

  int affectedDimension( -1 );
  int affectedSlice( -1 );
  if ( !SegTool2D::DetermineAffectedImageSlice( image, planeGeometry, affectedDimension, affectedSlice ) ) return false;
    
  Point3D worldCoordinates = positionEvent->GetWorldPosition();
  Point3D indexCoordinates;
  image->GetGeometry()->WorldToIndex( worldCoordinates, indexCoordinates );

  unsigned int firstDimension(0);
  unsigned int secondDimension(1);
  switch( affectedDimension )
  {
    case 2: // transversal
    default:
      firstDimension = 0;
      secondDimension = 1;
      break;
    case 1: // frontal
      firstDimension = 0;
      secondDimension = 2;
      break;
    case 0: // sagittal
      firstDimension = 1;
      secondDimension = 2;
      break;
  }

  // round to nearest voxel center (abort if this hasn't changed)
  if ( m_Size % 2 == 0 ) // even
  {
    indexCoordinates[firstDimension] = ROUND( indexCoordinates[firstDimension] + 0.5 );
    indexCoordinates[secondDimension] = ROUND( indexCoordinates[secondDimension] + 0.5 );
  }
  else // odd
  {
    indexCoordinates[firstDimension] = ROUND( indexCoordinates[firstDimension] );
    indexCoordinates[secondDimension] = ROUND( indexCoordinates[secondDimension] );
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
    //std::cout << "." << std::flush;
    return false;
  }

  Contour::Pointer contour = Contour::New();
  contour->Initialize();
  for (unsigned int index = 0; index < m_MasterContour->GetNumberOfPoints(); ++index)
  {
    Point3D point = m_MasterContour->GetPoints()->ElementAt(index);
    point[0] += indexCoordinates[ firstDimension ];
    point[1] += indexCoordinates[ secondDimension ];

    contour->AddVertex( point );
  }
  
  Image::Pointer slice = SegTool2D::GetAffectedImageSliceAs2DImage( positionEvent, image );
  if ( slice.IsNull() ) return false;

  if (leftMouseButtonPressed)
  {
    FeedbackContourTool::FillContourInSlice( contour, slice, m_PaintingPixelValue );

    OverwriteSliceImageFilter::Pointer slicewriter = OverwriteSliceImageFilter::New();
    slicewriter->SetInput( image );
    slicewriter->SetCreateUndoInformation( true );
    slicewriter->SetSliceImage( slice );
    slicewriter->SetSliceDimension( affectedDimension );
    slicewriter->SetSliceIndex( affectedSlice );
    slicewriter->SetTimeStep( positionEvent->GetSender()->GetTimeStep( image ) );
    slicewriter->Update();
  }

  // visualize contour
  Contour::Pointer displayContour = Contour::New();
  displayContour->Initialize();
  for (unsigned int index = 0; index < contour->GetNumberOfPoints(); ++index)
  {
    Point3D point = contour->GetPoints()->ElementAt(index);
    if ( m_Size % 2 != 0 ) // even
    {
      point[0] += 0.5;
      point[1] += 0.5;
    }

    displayContour->AddVertex( point );
  }

  displayContour = FeedbackContourTool::BackProjectContourFrom2DSlice( slice, displayContour );
  SetFeedbackContour( *displayContour );
  assert( positionEvent->GetSender()->GetRenderWindow() );

  RenderingManager::GetInstance()->RequestUpdate( positionEvent->GetSender()->GetRenderWindow() );

  return true;
}


bool mitk::PaintbrushTool::OnMouseReleased(Action* /*action*/, const StateEvent* /*stateEvent*/)
{
  //FeedbackContourTool::SetFeedbackContourVisible(false);

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

