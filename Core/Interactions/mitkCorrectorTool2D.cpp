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

#include "mitkCorrectorTool2D.h"

#include "mitkToolManager.h"
#include "mitkOverwriteSliceImageFilter.h"
#include "mitkBaseRenderer.h"
#include "mitkRenderingManager.h"

#include "mitkCorrectorTool2D.xpm"

mitk::CorrectorTool2D::CorrectorTool2D(int paintingPixelValue)
:SegTool2D("PressMoveRelease"),
 m_PaintingPixelValue(paintingPixelValue)
{
  GetFeedbackContour()->SetClosed( false ); // don't close the contour to a polygon
}

mitk::CorrectorTool2D::~CorrectorTool2D()
{
}

const char** mitk::CorrectorTool2D::GetXPM() const
{
  return mitkCorrectorTool2D_xpm;
}

const char* mitk::CorrectorTool2D::GetName() const
{
  return "Correction";
}

void mitk::CorrectorTool2D::Activated()
{
  Superclass::Activated();
}

void mitk::CorrectorTool2D::Deactivated()
{
  Superclass::Deactivated();
}

bool mitk::CorrectorTool2D::OnMousePressed (Action* action, const StateEvent* stateEvent)
{
  if (!SegTool2D::OnMousePressed( action, stateEvent )) return false;

  const PositionEvent* positionEvent = dynamic_cast<const PositionEvent*>(stateEvent->GetEvent());
  if (!positionEvent) return false;

  Contour* contour = SegTool2D::GetFeedbackContour();
  contour->Initialize();
  contour->AddVertex( positionEvent->GetWorldPosition() );
  
  SegTool2D::SetFeedbackContourVisible(true);

  return true;
}

bool mitk::CorrectorTool2D::OnMouseMoved   (Action* action, const StateEvent* stateEvent)
{
  if (!SegTool2D::OnMouseMoved( action, stateEvent )) return false;

  const PositionEvent* positionEvent = dynamic_cast<const PositionEvent*>(stateEvent->GetEvent());
  if (!positionEvent) return false;

  Contour* contour = SegTool2D::GetFeedbackContour();
  contour->AddVertex( positionEvent->GetWorldPosition() );

  assert( positionEvent->GetSender()->GetRenderWindow() );
  mitk::RenderingManager::GetInstance()->RequestUpdate( positionEvent->GetSender()->GetRenderWindow() );

  return true;
}

bool mitk::CorrectorTool2D::OnMouseReleased(Action* action, const StateEvent* stateEvent)
{
  // 1. Hide the feedback contour, find out which slice the user clicked, find out which slice of the toolmanager's working image corresponds to that
  SegTool2D::SetFeedbackContourVisible(false);
  
  const PositionEvent* positionEvent = dynamic_cast<const PositionEvent*>(stateEvent->GetEvent());
  if (!positionEvent) return false;

  assert( positionEvent->GetSender()->GetRenderWindow() );
  mitk::RenderingManager::GetInstance()->RequestUpdate( positionEvent->GetSender()->GetRenderWindow() );
  
  if (!SegTool2D::OnMouseReleased( action, stateEvent )) return false;

  DataTreeNode* workingNode( m_ToolManager->GetWorkingData(0) );
  if (!workingNode) return false;

  Image* image = dynamic_cast<Image*>(workingNode->GetData());
  const PlaneGeometry* planeGeometry( dynamic_cast<const PlaneGeometry*> (positionEvent->GetSender()->GetCurrentWorldGeometry2D() ) );
  if ( !image || !planeGeometry ) return false;

  int affectedDimension( -1 );
  int affectedSlice( -1 );
  if ( SegTool2D::DetermineAffectedImageSlice( image, planeGeometry, affectedDimension, affectedSlice ) )
  {
    // 2. Slice is known, now we try to get it as a 2D image and project the contour into index coordinates of this slice
    m_WorkingSlice = SegTool2D::GetAffectedImageSliceAs2DImage( positionEvent, image );

    if ( m_WorkingSlice.IsNull() )
    {
      std::cerr << "Unable to extract slice." << std::endl;
      return false;
    }
    
    // Convert to ipMITKSegmentationTYPE (because TobiasHeimannCorrectionAlgorithm relys on that data type)
    itk::Image< ipMITKSegmentationTYPE, 2 >::Pointer correctPixelTypeImage;
    CastToItkImage( m_WorkingSlice, correctPixelTypeImage );
    assert (correctPixelTypeImage.IsNotNull() );

  // possible bug in CastToItkImage ?
  // direction maxtrix is wrong/broken/not working after CastToItkImage, leading to a failed assertion in
  // mitk/Core/DataStructures/mitkSlicedGeometry3D.cpp, 479:
  // virtual void mitk::SlicedGeometry3D::SetSpacing(const mitk::Vector3D&): Assertion `aSpacing[0]>0 && aSpacing[1]>0 && aSpacing[2]>0' failed
  // solution here: we overwrite it with an unity matrix
  itk::Image< ipMITKSegmentationTYPE, 2 >::DirectionType imageDirection;
  imageDirection.SetIdentity();
  correctPixelTypeImage->SetDirection(imageDirection);

    Image::Pointer temporarySlice = Image::New();
  //  temporarySlice = ImportItkImage( correctPixelTypeImage );
    CastToMitkImage( correctPixelTypeImage, temporarySlice );

    // Use the correction algorithm instead of contour filling
    TobiasHeimannCorrectionAlgorithm( temporarySlice->GetSliceData()->GetPicDescriptor() );

    // 5. Write the modified 2D working data slice back into the image
    OverwriteSliceImageFilter::Pointer slicewriter = OverwriteSliceImageFilter::New();
    slicewriter->SetInput( image );
    slicewriter->SetCreateUndoInformation( true );
    slicewriter->SetSliceImage( temporarySlice );
    slicewriter->SetSliceDimension( affectedDimension );
    slicewriter->SetSliceIndex( affectedSlice );
    slicewriter->SetTimeStep( positionEvent->GetSender()->GetTimeStep( image ) );
    slicewriter->Update();

    // 6. Make sure the result is drawn again --> is visible then. 
    assert( positionEvent->GetSender()->GetRenderWindow() );
    mitk::RenderingManager::GetInstance()->RequestUpdate( positionEvent->GetSender()->GetRenderWindow() );
  }
  else
  {
    SliceBasedSegmentationBugMessage( "SegTool2D could not determine which slice of the image you are drawing on." );
  }

  return true;
}
    
void mitk::CorrectorTool2D::TobiasHeimannCorrectionAlgorithm(ipPicDescriptor* pic)
{
/*!
Some documentation (not by the original author)

TobiasHeimannCorrectionAlgorithm will be called, when the user has finished drawing a freehand line.

There should be different results, depending on the line's properties:

1. Without any prior segmentation, the start point and the end point of the drawn line will be 
connected to a contour and the area enclosed by the contour will be marked as segmentation.

2. When the whole line is inside a segmentation, start and end point will be connected to
a contour and the area of this contour will be substracted from the segmentation.

3. When the line starts inside a segmentation and ends outside with only a single
transition from segmentation to no-segmentation, nothing will happen.

4. When there are multiple transitions between inside-segmentation and
outside-segmentation, the line will be divided in so called segments. Each segment is
either fully inside or fully outside a segmentation. When it is inside a segmentation, its
enclosed area will be substracted from the segmentation. When the segment is outside a
segmentation, its enclosed area it will be added to the segmentation.

The algorithm is described in full length in Tobias Heimann's diploma thesis 
(MBI Technical Report 145, p. 37 - 40).
*/

  int oaSize = 1000000; // if we need a fixed number, then let it be big
  int* _ofsArray = new int[ oaSize ];
  for (int i=0; i<oaSize; i++) _ofsArray[i] = 0;

  std::vector<TSegData> segData;
  segData.reserve( 16 );
    
  Contour* feedbackContour( SegTool2D::GetFeedbackContour() );
  Contour::Pointer projectedContour = SegTool2D::ProjectContourTo2DSlice( m_WorkingSlice, feedbackContour, true, false ); 
      
  if (projectedContour.IsNull()) 
  {
    delete[] _ofsArray;
    return;
  }

  if (projectedContour->GetNumberOfPoints() < 2) 
  {
    delete[] _ofsArray;
    return;
  }
  
  // convert the projected contour into a ipSegmentation format
  ipInt4_t* _points = new ipInt4_t[2 * projectedContour->GetNumberOfPoints()];
  const Contour::PathType::VertexListType* pointsIn2D = projectedContour->GetContourPath()->GetVertexList();
  unsigned int index(0);
  for ( Contour::PathType::VertexListType::const_iterator iter = pointsIn2D->begin(); 
        iter != pointsIn2D->end();
        ++iter, ++index )
  {
    _points[ 2 * index + 0 ] = static_cast<ipInt4_t>( (*iter)[0] + 1.0 );
    _points[ 2 * index + 1 ] = static_cast<ipInt4_t>( (*iter)[1] + 1.0 );
  }
 
  // store ofsets of the drawn line in array
  int _ofsNum = 0;
  unsigned int num = projectedContour->GetNumberOfPoints();
  int lastOfs = -1;
  for (unsigned int i=0; i<num-1; i++) 
  {
    float x = _points [2*i] + 0.5;
    float y = _points [2*i+1] + 0.5;
    float difX = _points [2*i+2] - x + 0.5;
    float difY = _points [2*i+3] - y + 0.5;
    float length = sqrt( difX*difX + difY*difY );
    float dx = difX / length;
    float dy = difY / length;
    for (int p=0; ((float)p)<length; p++) 
    {
      // if ofs is out of bounds, very nasty things will happen, so better check coordinates:
      if (x<0) x=0.5;
      else if (x>=pic->n[0]) x = pic->n[0]-0.5;
      if (y<0) y=0.5;
      else if (y>=pic->n[1]) y = pic->n[1]-0.5;
      // ok, now store safe ofs
      int ofs = (int)(x) + pic->n[0]*((int)(y));
      x += dx;
      y += dy;
      if (ofs != lastOfs) 
      {
        _ofsArray[_ofsNum++] = ofs;
        lastOfs = ofs;
      }
    }
  }

  ipMITKSegmentationTYPE* picdata = static_cast<ipMITKSegmentationTYPE*>(pic->data);

  // divide line in logical segments:
  int numSegments = 0;
  ipMITKSegmentationTYPE state = *(picdata + _ofsArray[0]);
  int ofsP = 1;
  int modifyStart, modifyEnd;  // start of first and end of last segment
  bool nextSegment;
  segData.clear();
  do 
  {
    nextSegment = false;
    while (ofsP<_ofsNum && *(picdata + _ofsArray[ofsP])==state) ofsP++;
    if (ofsP<_ofsNum) 
    {
      int lineStart = ofsP-1;
      if (numSegments==0) modifyStart = ofsP;
      state = *(picdata + _ofsArray[ofsP]);
      while (ofsP<_ofsNum && *(picdata + _ofsArray[ofsP])==state) ofsP++;
      if (ofsP<_ofsNum) 
      {
        int lineEnd = ofsP;
        modifyEnd = lineEnd;
        nextSegment = true;
        // now we've got a valid segment from lineStart to lineEnd
        TSegData thisSegData;
        thisSegData.lineStart = lineStart;
        thisSegData.lineEnd = lineEnd;
        thisSegData.modified = modifySegment( lineStart, lineEnd, state, pic, _ofsArray );
        segData.push_back( thisSegData );
        numSegments++;
      }
    }
  } while (nextSegment);

  for (int segNr=0; segNr < numSegments; segNr++) 
  {
    // draw line if modified:
    if ( segData[segNr].modified ) 
    {
      for (int i=segData[segNr].lineStart+1; i<segData[segNr].lineEnd; i++) 
      {
        *(picdata + _ofsArray[i]) = 1;
      }
    }
  }

  if (numSegments == 0) 
  {
    if (num <= 1) 
    { // only a single pixel. _ofsArray[_ofsNum-1] in else statement would crash, so don't do anything
      // no movement: delete operation

      // This behaviour would probably confuse users when they use the correction 
      // tool to change a segmentation and it deletes much more than selected
     
      // if (state == 1)  ipMITKSegmentationReplaceRegion4N( pic, _ofsArray[0], 0 );
    }
    else if ( *(picdata + _ofsArray[_ofsNum-1]) == *(picdata + _ofsArray[0]))
    {
      // start point and end point both inside or both outside any segmentation
      // normal paint operation
      ipInt4_t* p = new ipInt4_t[2 * num];
      for (unsigned int i = 0; i < num; i++) 
      {
        p[2 * i] = (ipInt4_t) _points [2 * i];
        p[2 * i + 1] = (ipInt4_t) _points [2 * i + 1];
      }

      if (state == 0) ipMITKSegmentationCombineRegion (pic, p, num, 0, IPSEGMENTATION_OR,  1);
      else            ipMITKSegmentationCombineRegion (pic, p, num, 0, IPSEGMENTATION_AND, 0);

      delete[] p;
    }
  }

  int numberOfContourPoints( 0 );
  int oneContourOffset( 0 );
  int newBufferSize( 0 );

  int imageSize = pic->n[0]*pic->n[1];
  for (oneContourOffset = 0; oneContourOffset < imageSize; oneContourOffset++)
    if ( ((ipMITKSegmentationTYPE*) pic->data)[oneContourOffset]> 0) break;

  float* contourPoints = ipMITKSegmentationGetContour8N( pic, oneContourOffset, numberOfContourPoints, newBufferSize ); // memory allocated with malloc
  
  if (contourPoints) 
  {
    
    // copy point from float* to mitk::Contour 
    Contour::Pointer contourInImageIndexCoordinates = Contour::New();
    contourInImageIndexCoordinates->Initialize();
    Point3D newPoint;
    for (int index = 0; index < numberOfContourPoints; ++index)
    {
      newPoint[0] = contourPoints[ 2 * index + 0 ];
      newPoint[1] = contourPoints[ 2 * index + 1];
      newPoint[2] = 0;

      contourInImageIndexCoordinates->AddVertex( newPoint );
    }

    free(contourPoints);
    
    SegTool2D::FillContourInSlice( contourInImageIndexCoordinates, m_WorkingSlice );
  }
  
  delete[] _ofsArray;
  delete[] _points;
}

bool mitk::CorrectorTool2D::modifySegment( int lineStart, int lineEnd, ipMITKSegmentationTYPE state, ipPicDescriptor *pic, int* _ofsArray )
{
  // offsets for pixels right, top, left, bottom
  int nbDelta4[4];
  nbDelta4[0]=1;   nbDelta4[1]=pic->n[0];  nbDelta4[1]*=-1;  // necessary because of unsigned declaration of pic->n
  nbDelta4[2]=-1;  nbDelta4[3]=pic->n[0];

  // offsets for pixels right, top-right, top, top-left left, bottom-left, bottom, bottom-right
  int nbDelta8[8];
  nbDelta8[0] = 1;   nbDelta8[1] = nbDelta4[1]+1; nbDelta8[2] = nbDelta4[1];  nbDelta8[3] = nbDelta4[1]-1; 
  nbDelta8[4] = -1;  nbDelta8[5] = nbDelta4[3]-1; nbDelta8[6] = nbDelta4[3];  nbDelta8[7] = nbDelta4[3]+1; 
  
  ipMITKSegmentationTYPE* picdata = static_cast<ipMITKSegmentationTYPE*>(pic->data);

  ipMITKSegmentationTYPE saveStart = *(picdata + _ofsArray[lineStart]);
  ipMITKSegmentationTYPE saveEnd = *(picdata + _ofsArray[lineEnd]);
  ipMITKSegmentationTYPE newState = ((!state)&1) + 2; // probably equal to: ipMITKSegmentationTYPE newState = 3 - state; 

  // make two copies of pic:
  ipPicDescriptor *seg1 = ipPicClone( pic );
  ipPicDescriptor *seg2 = ipPicClone( pic );

  int i;

  // mark line in original
  for (i=lineStart; i<=lineEnd; i++) {
    *(picdata + _ofsArray[i]) = 3;
  }

  // mark the first side in copy 1:
  bool firstPix = true;
  bool modified;
  int line = pic->n[0];                 // #pixels in line
  int maxOfs = (int)(line * pic->n[1]); // #pixels in slice

  for (i=lineStart+1; i<lineEnd; i++) {
    do {
      modified = false;
      for (int nb=0; nb<8; nb++) {
        int nbOfs = _ofsArray[i] + nbDelta8[nb];
        if (   nbOfs < 0        // above first line
            || nbOfs >= maxOfs   // below last line
           ) continue;
        ipMITKSegmentationTYPE nbVal = *(picdata + nbOfs);
        ipMITKSegmentationTYPE destVal = *(((ipMITKSegmentationTYPE*)seg1->data) + nbOfs);
        if (nbVal!=3 && destVal!=newState) {  // get only neigbhours that are not part of the line itself
          if (firstPix) {
            *(((ipMITKSegmentationTYPE*)seg1->data) + nbOfs) = newState;  // this one is used to mark the side!
            firstPix = false;
            modified = true;
          }
          else {
            int tnb = 0;
            while (   tnb < 4 
                   && ((nbOfs + nbDelta4[tnb]) >= 0) &&  ((nbOfs + nbDelta4[tnb]) < maxOfs) 
                   && *(((ipMITKSegmentationTYPE*)seg1->data) + nbOfs + nbDelta4[tnb]) != newState
                  ) 
                  tnb++;

            if (tnb < 4 && ((nbOfs + nbDelta4[tnb]) >= 0) &&  ((nbOfs + nbDelta4[tnb]) < maxOfs) ) {
              *(((ipMITKSegmentationTYPE*)seg1->data) + nbOfs) = newState;  // we've got a buddy close
              modified = true;
            }
          }
        }
      }
    } while (modified);
  }

  // mark the other side in copy 2:
  for (i=lineStart+1; i<lineEnd; i++) {
    for (int nb=0; nb<4; nb++) {
      int nbOfs = _ofsArray[i] + nbDelta4[nb];
      if (   nbOfs < 0        // above first line
          || nbOfs >= maxOfs   // below last line
          ) continue;
      ipMITKSegmentationTYPE lineVal = *(picdata + nbOfs);
      ipMITKSegmentationTYPE side1Val = *(((ipMITKSegmentationTYPE*)seg1->data) + nbOfs);
      if (lineVal != 3 && side1Val != newState) {
        *(((ipMITKSegmentationTYPE*)seg2->data) + nbOfs) = newState;
      }
    }
  }

  // take care of line ends for multiple segments:
  *(((ipMITKSegmentationTYPE*)seg1->data) + _ofsArray[lineStart]) = newState;
  *(((ipMITKSegmentationTYPE*)seg1->data) + _ofsArray[lineEnd]) = newState;
  *(((ipMITKSegmentationTYPE*)seg2->data) + _ofsArray[lineStart]) = newState;
  *(((ipMITKSegmentationTYPE*)seg2->data) + _ofsArray[lineEnd]) = newState;


  // replace regions:
  newState = (!state)&1;
  int sizeRegion1 = 0, sizeRegion2 = 0;
  for (i=lineStart+1; i<lineEnd; i++) {
    if (*(((ipMITKSegmentationTYPE*)seg1->data) + _ofsArray[i]) != newState) {
      sizeRegion1 += ipMITKSegmentationReplaceRegion4N( seg1, _ofsArray[i], newState );
    }
    if (*(((ipMITKSegmentationTYPE*)seg2->data) + _ofsArray[i]) != newState) {
      sizeRegion2 += ipMITKSegmentationReplaceRegion4N( seg2, _ofsArray[i], newState );
    }
  }

  // combine image:
  //printf( "Size Region1 = %8i      Size Region2 = %8i\n", sizeRegion1, sizeRegion2 );
  int sizeDif;
  ipMITKSegmentationTYPE *current, *segSrc;
  if (sizeRegion1 < sizeRegion2) {
    segSrc = (ipMITKSegmentationTYPE*)seg1->data;
    sizeDif = sizeRegion2 - sizeRegion1;    
  }
  else {
    segSrc = (ipMITKSegmentationTYPE*)seg2->data;
    sizeDif = sizeRegion1 - sizeRegion2;
  }

  modified = false;
  if (sizeDif > 2*(lineEnd-lineStart)) {
    // decision is safe enough:
    ipMITKSegmentationTYPE *end = picdata + (pic->n[0]*pic->n[1]);
    for (current = picdata; current<end; current++) {
      if (*segSrc == newState) *current = newState;
      segSrc++;
    }
    modified = true;
  }

  // restore line:
  for (int i=lineStart+1; i<lineEnd; i++) {
    *(picdata + _ofsArray[i]) = state;
  }
  // restore end points:
  *(picdata + _ofsArray[lineStart]) = saveStart;
  *(picdata + _ofsArray[lineEnd]) = saveEnd;
  
  ipPicFree( seg1 );
  ipPicFree( seg2 );

  return modified;
}

