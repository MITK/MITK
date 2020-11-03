/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

// mitk headers
#include "mitkTargetPointsCalculator.h"
#include "mitkSurfaceToPointSetFilter.h"
#include <mitkImageCast.h>
#include <mitkSurfaceToImageFilter.h>

// itk headers
#include <itkImage.h>
#include <itkImageRegionIterator.h>

#define ROUND(a) ((a) > 0 ? (int)((a) + 0.5) : -(int)(0.5 - (a)))
typedef itk::Image<unsigned char, 3> ImageType;
typedef itk::ImageRegionIterator<ImageType> IteratorType;

mitk::TargetPointsCalculator::TargetPointsCalculator()
{
  // set default values
  m_InterPointDistance = 20;
  m_ErrorMessage = "";
  m_Method = mitk::TargetPointsCalculator::EvenlyDistributedTargetPoints;
}

mitk::TargetPointsCalculator::~TargetPointsCalculator()
{
}

void mitk::TargetPointsCalculator::SetInput(mitk::Surface::Pointer input)
{
  m_Input = input;
}

bool mitk::TargetPointsCalculator::DoCalculate()
{
  if (m_Input.IsNull())
  {
    m_ErrorMessage = "Error in TargetPointsCalculator: please set input first!";
    return false;
  }

  if (m_Method == mitk::TargetPointsCalculator::EvenlyDistributedTargetPoints)
  {
    mitk::Image::Pointer binaryImage = this->CreateBinaryImage();
    this->m_Output = this->CreateTargetPoints(binaryImage);
  }
  else if (m_Method == mitk::TargetPointsCalculator::OneTargetPointInCenterOfGravity)
  {
    this->m_Output = this->CreateTargetPointInCOG(m_Input);
  }
  else
  {
    return false;
  }

  return true;
}

void mitk::TargetPointsCalculator::SetTargetPointCalculationMethod(TargetPointCalculationMethod method)
{
  m_Method = method;
}

mitk::PointSet::Pointer mitk::TargetPointsCalculator::GetOutput()
{
  return m_Output;
}

std::string mitk::TargetPointsCalculator::GetErrorMessage()
{
  return m_ErrorMessage;
}

mitk::Image::Pointer mitk::TargetPointsCalculator::CreateBinaryImage()
{
  //################################################################################
  //###################### create binary image out of stl ##########################
  //################################################################################

  typedef unsigned char PixelType;

  // get bounding box of current surface
  const mitk::BoundingBox *boundingBox = this->m_Input->GetGeometry()->GetBoundingBox();
  mitk::BoundingBox::PointType minimum = boundingBox->GetMinimum();
  mitk::BoundingBox::PointType maximum = boundingBox->GetMaximum();

  // create white itk image
  ImageType::Pointer image = ImageType::New();
  ImageType::IndexType start;
  start[0] = 0;
  start[1] = 0;
  start[2] = 0;
  ImageType::SizeType imageSize;
  imageSize[0] = maximum[0] - minimum[0] + 20;
  imageSize[1] = maximum[1] - minimum[1] + 20;
  imageSize[2] = maximum[2] - minimum[2] + 20;
  ImageType::RegionType region;
  region.SetSize(imageSize);
  region.SetIndex(start);
  image->SetRegions(region);
  image->Allocate();

  // set all pixel values to 1
  PixelType pixel = 1;
  IteratorType iterator(image, image->GetRequestedRegion());
  iterator.GoToBegin();
  while (!iterator.IsAtEnd())
  {
    iterator.Set(pixel);
    ++iterator;
  }

  // convert to mitk image
  mitk::Image::Pointer mitkImage;
  mitk::CastToMitkImage(image, mitkImage);
  mitk::Point3D orig;
  orig[0] = minimum[0] - 10;
  orig[1] = minimum[1] - 10;
  orig[2] = minimum[2] - 10;
  mitkImage->SetOrigin(orig);
  mitkImage->UpdateOutputInformation();

  mitk::SurfaceToImageFilter::Pointer surfaceToImage = mitk::SurfaceToImageFilter::New();
  surfaceToImage->SetImage(mitkImage);
  surfaceToImage->SetInput(m_Input);
  surfaceToImage->MakeOutputBinaryOn();
  surfaceToImage->Update();

  return surfaceToImage->GetOutput();
}

mitk::PointSet::Pointer mitk::TargetPointsCalculator::CreateTargetPoints(mitk::Image::Pointer binaryImage)
{
  // determine bounding box:

  ImageType::Pointer itkImage = ImageType::New();
  mitk::CastToItkImage(binaryImage, itkImage);

  itk::Index<3> begin = {{0, 0, 0}};
  itk::Index<3> end = {{static_cast<signed long>(binaryImage->GetDimension(0)), static_cast<signed long>(binaryImage->GetDimension(1)), static_cast<signed long>(binaryImage->GetDimension(2))}};

  mitk::Point3D beginWorld;
  mitk::Point3D endWorld;

  itkImage->TransformIndexToPhysicalPoint(begin, beginWorld);
  itkImage->TransformIndexToPhysicalPoint(end, endWorld);
  // determine end of bounding box

  // Pointset initialisieren
  mitk::PointSet::Pointer returnValue = mitk::PointSet::New();
  int m = 0; // control value for Pointset-ID

  // initialize the distance of the points (in mm)
  int abstand = m_InterPointDistance;

  //#########################################################
  //############## calculation of the points####################
  //#########################################################

  // build up gate:
  mitk::Point3D p;
  for (int i = RoundUpToGatter(beginWorld.GetElement(0), abstand); i < endWorld.GetElement(0); i = i + abstand)
  {
    for (int j = RoundUpToGatter(beginWorld.GetElement(1), abstand); j < endWorld.GetElement(1); j = j + abstand)
    {
      for (int k = RoundUpToGatter(beginWorld.GetElement(2), abstand); k < endWorld.GetElement(2); k = k + abstand)
      {
        mitk::FillVector3D<mitk::Point3D>(p, i, j, k);

        // if it is inside the main structure
        if (this->isInside(itkImage, p))
        {
          returnValue->SetPoint(m, p);
          m++;
        }
      }
    }
  }

  return returnValue;
}

int mitk::TargetPointsCalculator::RoundUpToGatter(int i, int gatter)
{
  int centimeters = RoundUpToCentimeters(i);
  int mod = centimeters % gatter;
  int returnValue = centimeters + (gatter - mod);
  return returnValue;
}

int mitk::TargetPointsCalculator::RoundUpToCentimeters(int i)
{
  int returnValue = (i + 9.999) / 10;
  returnValue = returnValue * 10;
  return returnValue;
}

bool mitk::TargetPointsCalculator::isInside(ImageType::Pointer currentImageAsitkImage, mitk::Point3D p)
{
  itk::Index<3> contInd;

  if (currentImageAsitkImage->TransformPhysicalPointToIndex(p, contInd))
  {
    unsigned short pixel = currentImageAsitkImage->GetPixel(contInd);
    if (pixel == 1)
    {
      return true;
    }
    else
      return false;
  }
  else
    return false;
}

void mitk::TargetPointsCalculator::SetInterPointDistance(int d)
{
  this->m_InterPointDistance = d;
}

mitk::PointSet::Pointer mitk::TargetPointsCalculator::CreateTargetPointInCOG(mitk::Surface::Pointer surface)
{
  mitk::PointSet::Pointer returnValue = mitk::PointSet::New();

  // convert surface to point set
  mitk::SurfaceToPointSetFilter::Pointer mySurfaceToPointSetFilter = mitk::SurfaceToPointSetFilter::New();
  mySurfaceToPointSetFilter->SetInput(surface);
  mySurfaceToPointSetFilter->Update();
  mitk::PointSet::Pointer ptSet = mySurfaceToPointSetFilter->GetOutput();

  // calculate CoG
  mitk::Point3D CoG;
  CoG.Fill(0);
  for (int i = 0; i < ptSet->GetSize(); i++)
  {
    CoG[0] += ptSet->GetPoint(i)[0];
    CoG[1] += ptSet->GetPoint(i)[1];
    CoG[2] += ptSet->GetPoint(i)[2];
  }
  CoG[0] /= ptSet->GetSize();
  CoG[1] /= ptSet->GetSize();
  CoG[2] /= ptSet->GetSize();

  // update return value
  returnValue->InsertPoint(0, CoG);

  return returnValue;
}
