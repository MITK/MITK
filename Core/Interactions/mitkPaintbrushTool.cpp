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

int mitk::PaintbrushTool::m_Size = 10;

mitk::PaintbrushTool::PaintbrushTool(int paintingPixelValue)
:FeedbackContourTool("PressMoveReleaseWithCTRLInversion"),
 m_PaintingPixelValue(paintingPixelValue)
{
}

mitk::PaintbrushTool::~PaintbrushTool()
{
}

void mitk::PaintbrushTool::Activated()
{
  Superclass::Activated();
  SizeChanged.Send(m_Size);
}

void mitk::PaintbrushTool::Deactivated()
{
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

    contourInImageIndexCoordinates->AddVertex( newPoint );
  }

  free(contourPoints);

  m_MasterContour = FeedbackContourTool::BackProjectContourFrom2DSlice( m_WorkingSlice, contourInImageIndexCoordinates, true );   // true: sub 0.5 for ipSegmentation correctio

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

  this->OnMouseMoved(action, stateEvent);

  FeedbackContourTool::SetFeedbackContourVisible(true);

  return true;
}


/**
  Insert the point to the feedback contour,finish to build the contour and at the same time the painting function
  */
bool mitk::PaintbrushTool::OnMouseMoved   (Action* action, const StateEvent* stateEvent)
{
  if (!FeedbackContourTool::OnMouseMoved( action, stateEvent )) return false;

  const PositionEvent* positionEvent = dynamic_cast<const PositionEvent*>(stateEvent->GetEvent());
  if (!positionEvent) return false;

  DataTreeNode* workingNode( m_ToolManager->GetWorkingData(0) );
  if (!workingNode) return false;

  Image* image = dynamic_cast<Image*>(workingNode->GetData());
  const PlaneGeometry* planeGeometry( dynamic_cast<const PlaneGeometry*> (positionEvent->GetSender()->GetCurrentWorldGeometry2D() ) );
  if ( !image || !planeGeometry ) return false;

  int affectedDimension( -1 );
  int affectedSlice( -1 );
  if ( SegTool2D::DetermineAffectedImageSlice( image, planeGeometry, affectedDimension, affectedSlice ) )
  {
    // copy the master contour, but center it around current pixel
    Contour* contour = FeedbackContourTool::GetFeedbackContour();
    contour->Initialize();

    Point3D pos = positionEvent->GetWorldPosition();
    for (unsigned int index = 0; index < m_MasterContour->GetNumberOfPoints(); ++index)
    {
      Point3D point = m_MasterContour->GetPoints()->ElementAt(index);
      switch(affectedDimension)
      {
        case 2:
          point[0] += pos[0] + image->GetGeometry()->GetSpacing()[0] / 2.0; /* plus half a pixel's spacing */
          point[1] += pos[1] + image->GetGeometry()->GetSpacing()[1] / 2.0;
          break;
        case 1:
          point[0] += pos[0] + image->GetGeometry()->GetSpacing()[0] / 2.0;
          point[2] += pos[2] + image->GetGeometry()->GetSpacing()[2] / 2.0;
          break;
        case 0:
          point[1] += pos[1] + image->GetGeometry()->GetSpacing()[1] / 2.0;
          point[2] += pos[2] + image->GetGeometry()->GetSpacing()[2] / 2.0;
          break;
      }

      contour->AddVertex( point );
    }

    Image::Pointer slice = SegTool2D::GetAffectedImageSliceAs2DImage( positionEvent, image );

    if ( slice.IsNull() ) return false;

    Contour* feedbackContour( FeedbackContourTool::GetFeedbackContour() );
    Contour::Pointer projectedContour = FeedbackContourTool::ProjectContourTo2DSlice( slice, feedbackContour, true, false ); 

    if (projectedContour.IsNull()) return false;

    FeedbackContourTool::FillContourInSlice( projectedContour, slice, m_PaintingPixelValue );

    OverwriteSliceImageFilter::Pointer slicewriter = OverwriteSliceImageFilter::New();
    slicewriter->SetInput( image );
    slicewriter->SetCreateUndoInformation( true );
    slicewriter->SetSliceImage( slice );
    slicewriter->SetSliceDimension( affectedDimension );
    slicewriter->SetSliceIndex( affectedSlice );
    slicewriter->SetTimeStep( positionEvent->GetSender()->GetTimeStep( image ) );
    slicewriter->Update();

    assert( positionEvent->GetSender()->GetRenderWindow() );

    RenderingManager::GetInstance()->RequestUpdate( positionEvent->GetSender()->GetRenderWindow() );
  }

  return true;
}


bool mitk::PaintbrushTool::OnMouseReleased(Action* action, const StateEvent* stateEvent)
{
  FeedbackContourTool::SetFeedbackContourVisible(false);

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

