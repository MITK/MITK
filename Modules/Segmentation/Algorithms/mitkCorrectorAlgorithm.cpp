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

#include "mitkCorrectorAlgorithm.h"
#include "mitkImageCast.h"
#include "mitkImageAccessByItk.h"
#include "mitkImageDataItem.h"
#include "mitkContourUtils.h"

mitk::CorrectorAlgorithm::CorrectorAlgorithm()
:ImageToImageFilter()
{
}

mitk::CorrectorAlgorithm::~CorrectorAlgorithm()
{
}


void mitk::CorrectorAlgorithm::GenerateData()
{
  Image::Pointer inputImage = const_cast<Image*>(ImageToImageFilter::GetInput(0));

  if (inputImage.IsNull() || inputImage->GetDimension() != 2)
  {
    itkExceptionMacro("CorrectorAlgorithm needs a 2D image as input.");
  }

  if (m_Contour.IsNull())
  {
    itkExceptionMacro("CorrectorAlgorithm needs a Contour object as input.");
  }

   // copy the input (since m_WorkingImage will be changed later)
  m_WorkingImage = Image::New();
  m_WorkingImage->Initialize( inputImage );
  m_WorkingImage->SetVolume( inputImage.GetPointer()->GetData() );

  TimeSlicedGeometry::Pointer originalGeometry;

  if (inputImage->GetTimeSlicedGeometry() )
  {
    originalGeometry = inputImage->GetTimeSlicedGeometry()->Clone();
    m_WorkingImage->SetGeometry( originalGeometry );
  }
  else
  {
    itkExceptionMacro("Original image does not have a 'Time sliced geometry'! Cannot copy.");
  }

  Image::Pointer temporarySlice;
  // Convert to ipMITKSegmentationTYPE (because TobiasHeimannCorrectionAlgorithm relys on that data type)
  {
    itk::Image< ipMITKSegmentationTYPE, 2 >::Pointer correctPixelTypeImage;
    CastToItkImage( m_WorkingImage, correctPixelTypeImage );
    assert (correctPixelTypeImage.IsNotNull() );

    // possible bug in CastToItkImage ?
    // direction maxtrix is wrong/broken/not working after CastToItkImage, leading to a failed assertion in
    // mitk/Core/DataStructures/mitkSlicedGeometry3D.cpp, 479:
    // virtual void mitk::SlicedGeometry3D::SetSpacing(const mitk::Vector3D&): Assertion `aSpacing[0]>0 && aSpacing[1]>0 && aSpacing[2]>0' failed
    // solution here: we overwrite it with an unity matrix
    itk::Image< ipMITKSegmentationTYPE, 2 >::DirectionType imageDirection;
    imageDirection.SetIdentity();
    //correctPixelTypeImage->SetDirection(imageDirection);

    temporarySlice = this->GetOutput();
    //  temporarySlice = ImportItkImage( correctPixelTypeImage );
    CastToMitkImage( correctPixelTypeImage, temporarySlice );
  }


  mitkIpPicDescriptor* temporarySlicePic = mitkIpPicNew();
  CastToIpPicDescriptor( temporarySlice, temporarySlicePic );
  TobiasHeimannCorrectionAlgorithm( temporarySlicePic );

  temporarySlice->SetGeometry(originalGeometry);

  // temporarySlice is our return value (user  can get it by calling GetOutput() )

//  CalculateDifferenceImage( temporarySlice, inputImage );
//  if ( m_DifferenceImage.IsNotNull() && inputImage->GetTimeSlicedGeometry() )
//  {
//    AffineGeometryFrame3D::Pointer originalGeometryAGF = inputImage->GetTimeSlicedGeometry()->Clone();
//    TimeSlicedGeometry::Pointer originalGeometry = dynamic_cast<TimeSlicedGeometry*>( originalGeometryAGF.GetPointer() );
//    m_DifferenceImage->SetGeometry( originalGeometry );
//  }
//  else
//  {
//    itkExceptionMacro("Original image does not have a 'Time sliced geometry'! Cannot copy.");
//  }
}

void mitk::CorrectorAlgorithm::TobiasHeimannCorrectionAlgorithm(mitkIpPicDescriptor* pic)
{
/*!
Some documentation (not by the original author)

TobiasHeimannCorrectionAlgorithm will be called, when the user has finished drawing a freehand line.

There should be different results, depending on the line's properties:

1. Without any prior segmentation, the start point and the end point of the drawn line will be
connected to a contour and the area enclosed by the contour will be marked as segmentation.

2. When the whole line is inside a segmentation, start and end point will be connected to
a contour and the area of this contour will be subtracted from the segmentation.

3. When the line starts inside a segmentation and ends outside with only a single
transition from segmentation to no-segmentation, nothing will happen.

4. When there are multiple transitions between inside-segmentation and
outside-segmentation, the line will be divided in so called segments. Each segment is
either fully inside or fully outside a segmentation. When it is inside a segmentation, its
enclosed area will be subtracted from the segmentation. When the segment is outside a
segmentation, its enclosed area it will be added to the segmentation.

The algorithm is described in full length in Tobias Heimann's diploma thesis
(MBI Technical Report 145, p. 37 - 40).
*/

  int oaSize = 1000000; // if we need a fixed number, then let it be big
  int* _ofsArray = new int[ oaSize ];
  for (int i=0; i<oaSize; i++) _ofsArray[i] = 0;

  std::vector<TSegData> segData;
  segData.reserve( 16 );

  Contour* contour3D = const_cast<Contour*>(m_Contour.GetPointer());
  ContourUtils::Pointer contourUtils = ContourUtils::New();
  Contour::Pointer projectedContour = contourUtils->ProjectContourTo2DSlice( m_WorkingImage, contour3D, true, false );

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
  mitkIpInt4_t* _points = new mitkIpInt4_t[2 * projectedContour->GetNumberOfPoints()];
  const Contour::PathType::VertexListType* pointsIn2D = projectedContour->GetContourPath()->GetVertexList();
  unsigned int index(0);
  for ( Contour::PathType::VertexListType::const_iterator iter = pointsIn2D->begin();
        iter != pointsIn2D->end();
        ++iter, ++index )
  {
    _points[ 2 * index + 0 ] = static_cast<mitkIpInt4_t>( (*iter)[0] + 0.5 );
    _points[ 2 * index + 1 ] = static_cast<mitkIpInt4_t>( (*iter)[1] + 0.5 );
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

  if (_ofsNum == 0)
  {
    // contour was completely outside the binary image
    delete[] _ofsArray;
    delete[] _points;
    return;
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
      mitkIpInt4_t* p = new mitkIpInt4_t[2 * num];
      for (unsigned int i = 0; i < num; i++)
      {
        p[2 * i] = (mitkIpInt4_t) _points [2 * i];
        p[2 * i + 1] = (mitkIpInt4_t) _points [2 * i + 1];
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

    ContourUtils::Pointer contourUtils = ContourUtils::New();
    contourUtils->FillContourInSlice( contourInImageIndexCoordinates, m_WorkingImage );
  }

  delete[] _ofsArray;
  delete[] _points;
}

bool mitk::CorrectorAlgorithm::modifySegment( int lineStart, int lineEnd, ipMITKSegmentationTYPE state, mitkIpPicDescriptor *pic, int* _ofsArray )
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
  mitkIpPicDescriptor *seg1 = mitkIpPicClone( pic );
  mitkIpPicDescriptor *seg2 = mitkIpPicClone( pic );

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

  mitkIpPicFree( seg1 );
  mitkIpPicFree( seg2 );

  return modified;
}

void mitk::CorrectorAlgorithm::CalculateDifferenceImage( Image* modifiedImage, Image* originalImage )
{
  /*
   * modifiedImage has ipMITKSegmentationTYPE
   * originalImage may be any type
   *
   * --> we calculate a diff image using ITK, switching for the correct type of originalImage
   */
  m_DifferenceImage = NULL;
  Image::Pointer tmpPtr = originalImage;
  AccessFixedDimensionByItk_1( tmpPtr, ItkCalculateDifferenceImage, 2, modifiedImage );
}

template<typename TPixel, unsigned int VImageDimension>
void mitk::CorrectorAlgorithm::ItkCalculateDifferenceImage( itk::Image<TPixel, VImageDimension>* originalImage, Image* modifiedMITKImage )
{
  typedef itk::Image<ipMITKSegmentationTYPE, VImageDimension>                     ModifiedImageType;
  typedef itk::Image<short signed int, VImageDimension>                           DiffImageType;
  typedef itk::ImageRegionConstIterator< itk::Image<TPixel,VImageDimension> >     OriginalSliceIteratorType;
  typedef itk::ImageRegionConstIterator< ModifiedImageType >                      ModifiedSliceIteratorType;
  typedef itk::ImageRegionIterator< DiffImageType >                               DiffSliceIteratorType;

  typename ModifiedImageType::Pointer modifiedImage;
  CastToItkImage( modifiedMITKImage, modifiedImage );

  // create new image as a copy of the input
  // this new image is the output of this filter class
  typename DiffImageType::Pointer diffImage;
  m_DifferenceImage = Image::New();
  PixelType pixelType( mitk::MakeScalarPixelType<short>() );
  m_DifferenceImage->Initialize( pixelType, 2, modifiedMITKImage->GetDimensions() );
  CastToItkImage( m_DifferenceImage, diffImage );

  // iterators over both input images (original and modified) and the output image (diff)
  ModifiedSliceIteratorType modifiedIterator( modifiedImage, diffImage->GetLargestPossibleRegion() );
  OriginalSliceIteratorType originalIterator( originalImage, diffImage->GetLargestPossibleRegion() );
  DiffSliceIteratorType         diffIterator( diffImage,     diffImage->GetLargestPossibleRegion() );

  modifiedIterator.GoToBegin();
  originalIterator.GoToBegin();
  diffIterator.GoToBegin();

  while ( !diffIterator.IsAtEnd() )
  {
    short signed int difference = static_cast<short signed int>( static_cast<signed int>(modifiedIterator.Get()) -
                                                          static_cast<signed int>(originalIterator.Get())); // not good for bigger values ?!
    diffIterator.Set( difference );

    ++modifiedIterator;
    ++originalIterator;
    ++diffIterator;
  }
}

