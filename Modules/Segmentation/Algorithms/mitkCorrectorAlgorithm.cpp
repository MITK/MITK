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
#include "mitkLegacyAdaptors.h"

#include <mitkContourModelUtils.h>


#include "itkImageDuplicator.h"
#include "itkImageRegionIterator.h"


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
  m_WorkingImage = inputImage;

  TimeGeometry::Pointer originalGeometry = NULL;

  if (inputImage->GetTimeGeometry() )
  {
    originalGeometry = inputImage->GetTimeGeometry()->Clone();
    m_WorkingImage->SetTimeGeometry( originalGeometry );
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
    m_FillColor = 1;
    m_EraseColor = 0;
    ImprovedHeimannCorrectionAlgorithm(correctPixelTypeImage);
    CastToMitkImage( correctPixelTypeImage, temporarySlice );
  }
  temporarySlice->SetTimeGeometry(originalGeometry);
}

bool mitk::CorrectorAlgorithm::ImprovedHeimannCorrectionAlgorithm(itk::Image< ipMITKSegmentationTYPE, 2 >::Pointer pic)
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

  ContourModel::Pointer projectedContour = mitk::ContourModelUtils::ProjectContourTo2DSlice( m_WorkingImage, m_Contour, true, false );

  bool firstPointIsFillingColor = false;

  if (projectedContour.IsNull() ||
    projectedContour->GetNumberOfVertices() < 2 )
  {
    return false;
  }

  // Read the first point of the contour
  ContourModel::VertexIterator contourIter = projectedContour->Begin();
  if (contourIter == projectedContour->End())
    return false;
  itk::Index<2> previousIndex;
  previousIndex[0] = (*contourIter)->Coordinates[0];
  previousIndex[1] = (*contourIter)->Coordinates[1];
  ++contourIter;

  int currentColor = ( pic->GetPixel(previousIndex) == m_FillColor);
  firstPointIsFillingColor = currentColor;
  TSegData currentSegment;
  int countOfSegments = 1;


  bool firstSegment = true;
  ContourModel::VertexIterator contourEnd = projectedContour->End();
  for (; contourIter != contourEnd; ++contourIter)
  {
    // Get current point
    itk::Index<2> currentIndex;
    currentIndex[0] = (*contourIter)->Coordinates[0] +0.5;
    currentIndex[1] = (*contourIter)->Coordinates[1] +0.5;

    // Calculate length and slope
    double slopeX = currentIndex[0] - previousIndex[0];
    double slopeY = currentIndex[1] - previousIndex[1];
    double length = std::sqrt(slopeX * slopeX + slopeY * slopeY);
    double deltaX = slopeX / length;
    double deltaY = slopeY / length;

    for (double i = 0; i <= length && length > 0; i+=1)
    {
      itk::Index<2> temporaryIndex;
      temporaryIndex[0] = previousIndex[0] + deltaX * i;
      temporaryIndex[1] = previousIndex[1] + deltaY * i;
      if ( ! pic->GetLargestPossibleRegion().IsInside(temporaryIndex))
        continue;
      if ( (pic->GetPixel(temporaryIndex) == m_FillColor) != currentColor)
      {
        currentSegment.points.push_back(temporaryIndex);
        if ( ! firstSegment)
        {
          ModifySegment( currentSegment, pic);
        } else
        {
          firstSegment = false;
        }
        currentSegment = TSegData();
        ++countOfSegments;
        currentColor = (pic->GetPixel(temporaryIndex) == m_FillColor);
      }
      currentSegment.points.push_back(temporaryIndex);
    }
    previousIndex = currentIndex;
  }

  // Check if only on Segment
  if (firstSegment && currentSegment.points.size() > 0)
  {
      ContourModel::Pointer projectedContour = mitk::ContourModelUtils::ProjectContourTo2DSlice( m_WorkingImage, m_Contour, true, false );
      projectedContour->Close();
      if (firstPointIsFillingColor)
      {
        ContourModelUtils::FillContourInSlice(projectedContour, 0, m_WorkingImage, m_EraseColor);
      } else
      {
        ContourModelUtils::FillContourInSlice(projectedContour, 0, m_WorkingImage, m_FillColor);
      }
  }
  return true;
}

static void ColorSegment(const mitk::CorrectorAlgorithm::TSegData &segment, itk::Image< ipMITKSegmentationTYPE, 2 >::Pointer pic, int fillColor, int eraseColor)
{
  int colorMode = (pic->GetPixel(segment.points[0]) == fillColor);
  int color = 0;
  if (colorMode)
    color = eraseColor;
  else
    color = fillColor;

  std::vector< itk::Index<2> >::const_iterator indexIterator;
  std::vector< itk::Index<2> >::const_iterator indexEnd;

  indexIterator = segment.points.begin();
  indexEnd = segment.points.end();

  for (; indexIterator != indexEnd; ++indexIterator)
  {
    pic->SetPixel(*indexIterator, color);
  }
}

static itk::Index<2> GetFirstPoint(const mitk::CorrectorAlgorithm::TSegData &segment, itk::Image< ipMITKSegmentationTYPE, 2 >::Pointer pic, int fillColor)
{
  int colorMode = (pic->GetPixel(segment.points[0]) == fillColor);

  std::vector< itk::Index<2> >::const_iterator indexIterator;
  std::vector< itk::Index<2> >::const_iterator indexEnd;

  indexIterator = segment.points.begin();
  indexEnd = segment.points.end();

  for (; indexIterator != indexEnd; ++indexIterator)
  {
    itk::Index<2> index;
    for (int xOffset = -1 ; xOffset < 2; ++xOffset)
    {
      for (int yOffset = -1 ; yOffset < 2; ++yOffset)
      {
        index[0] = (*indexIterator)[0] - xOffset;
        index[1] = (*indexIterator)[1] - yOffset;
        if ((pic->GetPixel(index) == fillColor) != colorMode)
        {
          return index;
        }
      }
    }
  }
  mitkThrow() << "No Starting point is found next to the curve.";
}

static int FillRegion(const itk::Index<2> startIndex, itk::Image< ipMITKSegmentationTYPE, 2 >::Pointer pic, int fillColor, int eraseColor)
{
  int numberOfPixel = 0;
  int mode = (pic->GetPixel(startIndex) == fillColor);
  int drawColor = fillColor;
  if (mode)
  {
    drawColor = eraseColor;
  }

  std::vector<itk::Index<2> > workPoints;
  workPoints.push_back(startIndex);
  while (workPoints.size() > 0)
  {
    itk::Index<2> currentIndex = workPoints.back();
    workPoints.pop_back();
    if ((pic->GetPixel(currentIndex) == fillColor) == mode)
      ++numberOfPixel;
    pic->SetPixel(currentIndex, drawColor);

    currentIndex[0] -= 1;
    if (pic->GetLargestPossibleRegion().IsInside(currentIndex) &&  (pic->GetPixel(currentIndex) == fillColor) == mode)
      workPoints.push_back(currentIndex);
    currentIndex[0] += 2;
    if (pic->GetLargestPossibleRegion().IsInside(currentIndex) &&  (pic->GetPixel(currentIndex) == fillColor) == mode)
      workPoints.push_back(currentIndex);
    currentIndex[0] -= 1;
    currentIndex[1] -= 1;
    if (pic->GetLargestPossibleRegion().IsInside(currentIndex) &&  (pic->GetPixel(currentIndex) == fillColor) == mode)
      workPoints.push_back(currentIndex);
    currentIndex[1] += 2;
    if (pic->GetLargestPossibleRegion().IsInside(currentIndex) &&  (pic->GetPixel(currentIndex) == fillColor) == mode)
      workPoints.push_back(currentIndex);
  }
  return numberOfPixel;
}

static itk::Image< ipMITKSegmentationTYPE, 2 >::Pointer CloneImage(itk::Image< ipMITKSegmentationTYPE, 2 >::Pointer pic)
{
  typedef itk::Image< ipMITKSegmentationTYPE, 2 > ItkImageType;

  typedef itk::ImageDuplicator< ItkImageType > DuplicatorType;
  DuplicatorType::Pointer duplicator = DuplicatorType::New();
  duplicator->SetInputImage(pic);
  duplicator->Update();

  return duplicator->GetOutput();
}

static void OverwriteImage(itk::Image< ipMITKSegmentationTYPE, 2 >::Pointer source, itk::Image< ipMITKSegmentationTYPE, 2 >::Pointer target)
{
  typedef itk::Image< ipMITKSegmentationTYPE, 2 > ItkImageType;
  typedef itk::ImageRegionIterator<ItkImageType> ImageIteratorType;

  ImageIteratorType sourceIter(source, source->GetLargestPossibleRegion());
  ImageIteratorType targetIter(target, target->GetLargestPossibleRegion());
  while ( ! sourceIter.IsAtEnd())
  {
    targetIter.Set(sourceIter.Get());
    ++sourceIter;
    ++targetIter;
  }
}

bool  mitk::CorrectorAlgorithm::ModifySegment(const TSegData &segment, itk::Image< ipMITKSegmentationTYPE, 2 >::Pointer pic)
{
  try
  {
    typedef itk::Image< ipMITKSegmentationTYPE, 2 >::Pointer ItkImagePointerType;
    // Find the current ColorMode:

      // Kein erster Punkt
        // Erster Punkt finden
    ItkImagePointerType firstSideImage = CloneImage(pic);
    ColorSegment(segment, firstSideImage, m_FillColor, m_EraseColor);
    ItkImagePointerType secondSideImage = CloneImage(firstSideImage);
    itk::Index<2> firstPoint = GetFirstPoint(segment,  firstSideImage, m_FillColor);
    int firstSidePixel = FillRegion(firstPoint, firstSideImage, m_FillColor, m_EraseColor);

    // TODO  Problem: Was ist wenn keine Zweite Seite vorhanden?
    try
    {
      itk::Index<2> secondPoint = GetFirstPoint(segment, firstSideImage, m_FillColor);
      int secondSidePixel = FillRegion(secondPoint, secondSideImage, m_FillColor, m_EraseColor);

      if (firstSidePixel < secondSidePixel)
      {
        OverwriteImage(firstSideImage, pic);
      } else
      {
        OverwriteImage(secondSideImage, pic);
      }
    }
    catch (mitk::Exception e)
    {
      e.what();
    }
    return true;
  }
  catch(...)
  {
    return false;
  }
}