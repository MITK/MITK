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
#include "mitkITKImageImport.h"
#include "mitkImageDataItem.h"
#include "mitkContourUtils.h"
#include "mitkLegacyAdaptors.h"

#include <mitkContourModelUtils.h>

#include "itkImageDuplicator.h"
#include "itkImageRegionIterator.h"
#include "itkCastImageFilter.h"

mitk::CorrectorAlgorithm::CorrectorAlgorithm()
  :ImageToImageFilter()
  , m_FillColor(1)
  , m_EraseColor(0)
{
}

mitk::CorrectorAlgorithm::~CorrectorAlgorithm()
{
}

template<typename TPixel, unsigned int VDimensions>
void ConvertBackToCorrectPixelType(itk::Image< TPixel, VDimensions> * reference, mitk::Image::Pointer target, itk::Image< mitk::CorrectorAlgorithm::DefaultSegmentationDataType, 2 >::Pointer segmentationPixelTypeImage)
{
  typedef itk::Image< mitk::CorrectorAlgorithm::DefaultSegmentationDataType, 2 >   InputImageType;
  typedef itk::Image< TPixel, 2 >  OutputImageType;
  typedef itk::CastImageFilter< InputImageType, OutputImageType > CastImageFilterType;

  typename CastImageFilterType::Pointer castImageFilter = CastImageFilterType::New();
  castImageFilter->SetInput(segmentationPixelTypeImage);
  castImageFilter->Update();
  typename OutputImageType::Pointer tempItkImage = castImageFilter->GetOutput();
  tempItkImage->DisconnectPipeline();
  mitk::CastToMitkImage(tempItkImage, target);
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

  if (inputImage->GetTimeGeometry())
  {
    originalGeometry = inputImage->GetTimeGeometry()->Clone();
    m_WorkingImage->SetTimeGeometry(originalGeometry);
  }
  else
  {
    itkExceptionMacro("Original image does not have a 'Time sliced geometry'! Cannot copy.");
  }

  Image::Pointer temporarySlice;
  // Convert to DefaultSegmentationDataType (because TobiasHeimannCorrectionAlgorithm relys on that data type)
  {
    itk::Image< DefaultSegmentationDataType, 2 >::Pointer correctPixelTypeImage;
    CastToItkImage(m_WorkingImage, correctPixelTypeImage);
    assert(correctPixelTypeImage.IsNotNull());

    // possible bug in CastToItkImage ?
    // direction maxtrix is wrong/broken/not working after CastToItkImage, leading to a failed assertion in
    // mitk/Core/DataStructures/mitkSlicedGeometry3D.cpp, 479:
    // virtual void mitk::SlicedGeometry3D::SetSpacing(const mitk::Vector3D&): Assertion `aSpacing[0]>0 && aSpacing[1]>0 && aSpacing[2]>0' failed
    // solution here: we overwrite it with an unity matrix
    itk::Image< DefaultSegmentationDataType, 2 >::DirectionType imageDirection;
    imageDirection.SetIdentity();
    //correctPixelTypeImage->SetDirection(imageDirection);

    temporarySlice = this->GetOutput();
    //  temporarySlice = ImportItkImage( correctPixelTypeImage );
    ImprovedHeimannCorrectionAlgorithm(correctPixelTypeImage);

    //this is suboptimal, needs to be kept synchronous to DefaultSegmentationDataType
    if (inputImage->GetChannelDescriptor().GetPixelType().GetComponentType() == itk::ImageIOBase::USHORT)
    { //the cast at the beginning did not copy the data
      CastToMitkImage(correctPixelTypeImage, temporarySlice);
    }
    else
    { //it did copy the data and cast the pixel type
      AccessByItk_n(m_WorkingImage, ConvertBackToCorrectPixelType, (temporarySlice, correctPixelTypeImage));
    }

  }
  temporarySlice->SetTimeGeometry(originalGeometry);
}

template <typename ScalarType>
itk::Index<2> mitk::CorrectorAlgorithm::ensureIndexInImage(ScalarType i0, ScalarType i1)
{
  itk::Index<2> toReturn;

  itk::Size<5> size = m_WorkingImage->GetLargestPossibleRegion().GetSize();

  toReturn[0] = std::min((ScalarType)(size[0] - 1), std::max((ScalarType)0.0, i0));
  toReturn[1] = std::min((ScalarType)(size[1] - 1), std::max((ScalarType)0.0, i1));

  return toReturn;
}

bool mitk::CorrectorAlgorithm::ImprovedHeimannCorrectionAlgorithm(itk::Image< DefaultSegmentationDataType, 2 >::Pointer pic)
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

  ContourModel::Pointer projectedContour = mitk::ContourModelUtils::ProjectContourTo2DSlice(m_WorkingImage, m_Contour, true, false);

  bool firstPointIsFillingColor = false;

  if (projectedContour.IsNull() ||
    projectedContour->GetNumberOfVertices() < 2)
  {
    return false;
  }

  // Read the first point of the contour
  ContourModel::VertexIterator contourIter = projectedContour->Begin();
  if (contourIter == projectedContour->End())
    return false;
  itk::Index<2> previousIndex;

  previousIndex = ensureIndexInImage((*contourIter)->Coordinates[0], (*contourIter)->Coordinates[1]);
  ++contourIter;

  int currentColor = (pic->GetPixel(previousIndex) == m_FillColor);
  firstPointIsFillingColor = currentColor;
  TSegData currentSegment;
  int countOfSegments = 1;


  bool firstSegment = true;
  ContourModel::VertexIterator contourEnd = projectedContour->End();
  for (; contourIter != contourEnd; ++contourIter)
  {
    // Get current point
    itk::Index<2> currentIndex;
    currentIndex = ensureIndexInImage((*contourIter)->Coordinates[0] + 0.5, (*contourIter)->Coordinates[1] + 0.5);

    // Calculate length and slope
    double slopeX = currentIndex[0] - previousIndex[0];
    double slopeY = currentIndex[1] - previousIndex[1];
    double length = std::sqrt(slopeX * slopeX + slopeY * slopeY);
    double deltaX = slopeX / length;
    double deltaY = slopeY / length;

    for (double i = 0; i <= length && length > 0; i += 1)
    {
      itk::Index<2> temporaryIndex;
      temporaryIndex = ensureIndexInImage(previousIndex[0] + deltaX * i, previousIndex[1] + deltaY * i);

      if (!pic->GetLargestPossibleRegion().IsInside(temporaryIndex))
        continue;
      if ((pic->GetPixel(temporaryIndex) == m_FillColor) != currentColor)
      {
        currentSegment.points.push_back(temporaryIndex);
        if (!firstSegment)
        {
          ModifySegment(currentSegment, pic);
        }
        else
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
    ContourModel::Pointer projectedContour = mitk::ContourModelUtils::ProjectContourTo2DSlice(m_WorkingImage, m_Contour, true, false);
    projectedContour->Close();
    if (firstPointIsFillingColor)
    {
      ContourModelUtils::FillContourInSlice(projectedContour, 0, m_WorkingImage, m_EraseColor);
    }
    else
    {
      ContourModelUtils::FillContourInSlice(projectedContour, 0, m_WorkingImage, m_FillColor);
    }
  }
  return true;
}

void mitk::CorrectorAlgorithm::ColorSegment(const mitk::CorrectorAlgorithm::TSegData &segment, itk::Image< mitk::CorrectorAlgorithm::DefaultSegmentationDataType, 2 >::Pointer pic)
{
  int colorMode = (pic->GetPixel(segment.points[0]) == m_FillColor);
  int color = 0;
  if (colorMode)
    color = m_EraseColor;
  else
    color = m_FillColor;

  std::vector< itk::Index<2> >::const_iterator indexIterator;
  std::vector< itk::Index<2> >::const_iterator indexEnd;

  indexIterator = segment.points.begin();
  indexEnd = segment.points.end();

  for (; indexIterator != indexEnd; ++indexIterator)
  {
    pic->SetPixel(*indexIterator, color);
  }
}

itk::Image< mitk::CorrectorAlgorithm::DefaultSegmentationDataType, 2 >::Pointer mitk::CorrectorAlgorithm::CloneImage(itk::Image< mitk::CorrectorAlgorithm::DefaultSegmentationDataType, 2 >::Pointer pic)
{
  typedef itk::Image< mitk::CorrectorAlgorithm::DefaultSegmentationDataType, 2 > ItkImageType;

  typedef itk::ImageDuplicator< ItkImageType > DuplicatorType;
  DuplicatorType::Pointer duplicator = DuplicatorType::New();
  duplicator->SetInputImage(pic);
  duplicator->Update();

  return duplicator->GetOutput();
}

itk::Index<2> mitk::CorrectorAlgorithm::GetFirstPoint(const mitk::CorrectorAlgorithm::TSegData &segment, itk::Image< mitk::CorrectorAlgorithm::DefaultSegmentationDataType, 2 >::Pointer pic)
{
  int colorMode = (pic->GetPixel(segment.points[0]) == m_FillColor);

  std::vector< itk::Index<2> >::const_iterator indexIterator;
  std::vector< itk::Index<2> >::const_iterator indexEnd;

  indexIterator = segment.points.begin();
  indexEnd = segment.points.end();

  itk::Index<2> index;

  for (; indexIterator != indexEnd; ++indexIterator)
  {
    for (int xOffset = -1; xOffset < 2; ++xOffset)
    {
      for (int yOffset = -1; yOffset < 2; ++yOffset)
      {
        index = ensureIndexInImage((*indexIterator)[0] - xOffset, (*indexIterator)[1] - yOffset);

        if ((pic->GetPixel(index) == m_FillColor) != colorMode)
        {
          return index;
        }
      }
    }
  }
  mitkThrow() << "No Starting point is found next to the curve.";
}

std::vector<itk::Index<2> > mitk::CorrectorAlgorithm::FindSeedPoints(const mitk::CorrectorAlgorithm::TSegData &segment, itk::Image< mitk::CorrectorAlgorithm::DefaultSegmentationDataType, 2 >::Pointer pic)
{
  typedef itk::Image< mitk::CorrectorAlgorithm::DefaultSegmentationDataType, 2 > ItkImageType;
  typedef itk::Image< mitk::CorrectorAlgorithm::DefaultSegmentationDataType, 2 >::Pointer ItkImagePointerType;

  std::vector<itk::Index<2> > seedPoints;

  try
  {
    itk::Index<2> firstPoint = GetFirstPoint(segment, pic);
    seedPoints.push_back(firstPoint);
  }
  catch (mitk::Exception e)
  {
    return seedPoints;
  }

  if (segment.points.size() < 4)
    return seedPoints;

  std::vector< itk::Index<2> >::const_iterator indexIterator;
  std::vector< itk::Index<2> >::const_iterator indexEnd;

  indexIterator = segment.points.begin();
  indexEnd = segment.points.end();

  ItkImagePointerType listOfPoints = CloneImage(pic);
  listOfPoints->FillBuffer(0);
  listOfPoints->SetPixel(seedPoints[0], 1);
  for (; indexIterator != indexEnd; ++indexIterator)
  {
    listOfPoints->SetPixel(*indexIterator, 2);
  }
  indexIterator = segment.points.begin();
  indexIterator++;
  indexIterator++;
  indexEnd--;
  indexEnd--;
  for (; indexIterator != indexEnd; ++indexIterator)
  {
    bool pointFound = true;
    while (pointFound)
    {
      pointFound = false;
      itk::Index<2> index;
      itk::Index<2> index2;
      for (int xOffset = -1; xOffset < 2; ++xOffset)
      {
        for (int yOffset = -1; yOffset < 2; ++yOffset)
        {
          index = ensureIndexInImage((*indexIterator)[0] - xOffset, (*indexIterator)[1] - yOffset);
          index2 = index;

          if (listOfPoints->GetPixel(index2) > 0)
            continue;

          index[0] = index[0] - 1;
          index = ensureIndexInImage(index[0], index[1]);
          if (listOfPoints->GetPixel(index) == 1)
          {
            pointFound = true;
            seedPoints.push_back(index2);
            listOfPoints->SetPixel(index2, 1);
            continue;
          }

          index[0] = index[0] + 2;
          index = ensureIndexInImage(index[0], index[1]);
          if (listOfPoints->GetPixel(index) == 1)
          {
            pointFound = true;
            seedPoints.push_back(index2);
            listOfPoints->SetPixel(index2, 1);
            continue;
          }

          index[0] = index[0] - 1;
          index[1] = index[1] - 1;
          index = ensureIndexInImage(index[0], index[1]);
          if (listOfPoints->GetPixel(index) == 1)
          {
            pointFound = true;
            seedPoints.push_back(index2);
            listOfPoints->SetPixel(index2, 1);
            continue;
          }

          index[1] = index[1] + 2;
          index = ensureIndexInImage(index[0], index[1]);
          if (listOfPoints->GetPixel(index) == 1)
          {
            pointFound = true;
            seedPoints.push_back(index2);
            listOfPoints->SetPixel(index2, 1);
            continue;
          }
        }
      }
    }
  }
  return seedPoints;
}

int mitk::CorrectorAlgorithm::FillRegion(const std::vector<itk::Index<2> > &seedPoints, itk::Image< mitk::CorrectorAlgorithm::DefaultSegmentationDataType, 2 >::Pointer pic)
{
  int numberOfPixel = 0;
  int mode = (pic->GetPixel(seedPoints[0]) == m_FillColor);
  int drawColor = m_FillColor;
  if (mode)
  {
    drawColor = m_EraseColor;
  }

  std::vector<itk::Index<2> > workPoints;
  workPoints = seedPoints;
  //   workPoints.push_back(seedPoints[0]);
  while (workPoints.size() > 0)
  {
    itk::Index<2> currentIndex = workPoints.back();
    workPoints.pop_back();
    if ((pic->GetPixel(currentIndex) == m_FillColor) == mode)
      ++numberOfPixel;
    pic->SetPixel(currentIndex, drawColor);


    currentIndex = ensureIndexInImage(currentIndex[0] - 1, currentIndex[1]);
    if (pic->GetLargestPossibleRegion().IsInside(currentIndex) && (pic->GetPixel(currentIndex) == m_FillColor) == mode)
      workPoints.push_back(currentIndex);

    currentIndex = ensureIndexInImage(currentIndex[0] + 2, currentIndex[1]);
    if (pic->GetLargestPossibleRegion().IsInside(currentIndex) && (pic->GetPixel(currentIndex) == m_FillColor) == mode)
      workPoints.push_back(currentIndex);

    currentIndex = ensureIndexInImage(currentIndex[0] - 1, currentIndex[1] - 1);

    if (pic->GetLargestPossibleRegion().IsInside(currentIndex) && (pic->GetPixel(currentIndex) == m_FillColor) == mode)
      workPoints.push_back(currentIndex);

    currentIndex = ensureIndexInImage(currentIndex[0], currentIndex[1] + 2);
    if (pic->GetLargestPossibleRegion().IsInside(currentIndex) && (pic->GetPixel(currentIndex) == m_FillColor) == mode)
      workPoints.push_back(currentIndex);
  }

  return numberOfPixel;
}



void mitk::CorrectorAlgorithm::OverwriteImage(itk::Image< mitk::CorrectorAlgorithm::DefaultSegmentationDataType, 2 >::Pointer source, itk::Image< mitk::CorrectorAlgorithm::DefaultSegmentationDataType, 2 >::Pointer target)
{
  typedef itk::Image< mitk::CorrectorAlgorithm::DefaultSegmentationDataType, 2 > ItkImageType;
  typedef itk::ImageRegionIterator<ItkImageType> ImageIteratorType;

  ImageIteratorType sourceIter(source, source->GetLargestPossibleRegion());
  ImageIteratorType targetIter(target, target->GetLargestPossibleRegion());
  while (!sourceIter.IsAtEnd())
  {
    targetIter.Set(sourceIter.Get());
    ++sourceIter;
    ++targetIter;
  }
}

bool mitk::CorrectorAlgorithm::ModifySegment(const TSegData &segment, itk::Image< DefaultSegmentationDataType, 2 >::Pointer pic)
{
  typedef itk::Image< DefaultSegmentationDataType, 2 >::Pointer ItkImagePointerType;

  ItkImagePointerType firstSideImage = CloneImage(pic);
  ColorSegment(segment, firstSideImage);
  ItkImagePointerType secondSideImage = CloneImage(firstSideImage);

  std::vector<itk::Index<2> > seedPoints = FindSeedPoints(segment, firstSideImage);
  if (seedPoints.size() < 1)
    return false;
  int firstSidePixel = FillRegion(seedPoints, firstSideImage);

  std::vector<itk::Index<2> > secondSeedPoints = FindSeedPoints(segment, firstSideImage);
  if (secondSeedPoints.size() < 1)
    return false;
  int secondSidePixel = FillRegion(secondSeedPoints, secondSideImage);

  if (firstSidePixel < secondSidePixel)
  {
    OverwriteImage(firstSideImage, pic);
  }
  else
  {
    OverwriteImage(secondSideImage, pic);
  }
  return true;
}

