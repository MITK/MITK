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

#include "mitkToFDistanceImageToPointSetFilter.h"

#include "mitkImageDataItem.h"
#include "mitkPointSet.h"
#include <mitkImagePixelReadAccessor.h>
#include "mitkToFProcessingCommon.h"

mitk::ToFDistanceImageToPointSetFilter::ToFDistanceImageToPointSetFilter()
: m_Subset(NULL), m_CameraIntrinsics(), m_InterPixelDistance()
{
  m_InterPixelDistance.Fill(0.045);
  m_CameraIntrinsics = mitk::CameraIntrinsics::New();
  m_CameraIntrinsics->SetFocalLength(5.9421434211923247e+02,5.9104053696870778e+02);
  m_CameraIntrinsics->SetPrincipalPoint(3.3930780975300314e+02,2.4273913761751615e+02);
  m_CameraIntrinsics->SetDistorsionCoeffs(-0.36874385358645773f,-0.14339503290129013,0.0033210108720361795,-0.004277703352074105);
  m_ReconstructionMode = true;
}

mitk::ToFDistanceImageToPointSetFilter::~ToFDistanceImageToPointSetFilter()
{
}

void mitk::ToFDistanceImageToPointSetFilter::SetInput(const mitk::Image* distanceImage )
{
  this->SetInput(0,distanceImage);
}

void mitk::ToFDistanceImageToPointSetFilter::SetInput( unsigned int idx,const mitk::Image* distanceImage )
{
  if ((distanceImage == NULL) && (idx == this->GetNumberOfInputs() - 1)) // if the last input is set to NULL, reduce the number of inputs by one
  {
    this->SetNumberOfInputs(this->GetNumberOfInputs() - 1);
  }
  else
  {
    this->ProcessObject::SetNthInput(idx, const_cast<mitk::Image*>(distanceImage));   // Process object is not const-correct so the const_cast is required here
    this->CreateOutputsForAllInputs();
  }
}

mitk::Image* mitk::ToFDistanceImageToPointSetFilter::GetInput()
{
  return this->GetInput(0);
}

mitk::Image* mitk::ToFDistanceImageToPointSetFilter::GetInput( unsigned int idx )
{
  if (this->GetNumberOfInputs() < 1)
    return NULL;

  return static_cast< mitk::Image*>(this->ProcessObject::GetInput(idx));
}

void mitk::ToFDistanceImageToPointSetFilter::SetSubset(std::vector<itk::Index<3> > subset)
{
  // check if points of PointSet are inside the input image
  mitk::Image::Pointer input = this->GetInput();

  unsigned int xDim = UINT_MAX;
  unsigned int yDim = UINT_MAX;
  if(input.IsNotNull() && input->IsInitialized())
  {
    unsigned int xDim = input->GetDimension(0);
    unsigned int yDim = input->GetDimension(1);
  }

  bool pointSetValid = true;
  for (unsigned int i=0; i<subset.size(); i++)
  {
    itk::Index<3> currentIndex = subset.at(i);
    if (currentIndex[0]<0||currentIndex[0]>xDim||currentIndex[1]<0||currentIndex[1]>yDim)
    {
      pointSetValid = false;
    }
  }
  if (pointSetValid)
  {
    m_Subset = subset;
  }
  else
  {
    MITK_ERROR<<"One or more indizes are located outside the image domain";
  }
}

void mitk::ToFDistanceImageToPointSetFilter::SetSubset( mitk::PointSet::Pointer pointSet)
{
  std::vector<itk::Index<3> > subset;
  for (int i=0; i<pointSet->GetSize(); i++)
  {
    mitk::Point3D currentPoint = pointSet->GetPoint(i);
    itk::Index<3> currentIndex;
    currentIndex[0] = currentPoint[0];
    currentIndex[1] = currentPoint[1];
    currentIndex[2] = currentPoint[2];
    subset.push_back(currentIndex);
  }
  this->SetSubset(subset);
}

void mitk::ToFDistanceImageToPointSetFilter::GenerateData()
{
  //calculate world coordinates
  mitk::ToFProcessingCommon::ToFPoint2D focalLengthInPixelUnits;
  mitk::ToFProcessingCommon::ToFScalarType focalLengthInMm;
  if (m_ReconstructionMode)
  {
    focalLengthInPixelUnits[0] = m_CameraIntrinsics->GetFocalLengthX();
    focalLengthInPixelUnits[1] = m_CameraIntrinsics->GetFocalLengthY();
  }
  else
    focalLengthInMm = (m_CameraIntrinsics->GetFocalLengthX()*m_InterPixelDistance[0]+m_CameraIntrinsics->GetFocalLengthY()*m_InterPixelDistance[1])/2.0;

  mitk::ToFProcessingCommon::ToFPoint2D principalPoint;
  principalPoint[0] = m_CameraIntrinsics->GetPrincipalPointX();
  principalPoint[1] = m_CameraIntrinsics->GetPrincipalPointY();

  mitk::PointSet::Pointer output = this->GetOutput();
  assert(output);
  mitk::Image::Pointer input = this->GetInput();
  assert(input);

  //compute subset of points if input PointSet is defined
  if (m_Subset.size()!=0)
  {
    mitk::ImagePixelReadAccessor<float,2> imageAcces(input, input->GetSliceData(0));
    for (unsigned int i=0; i<m_Subset.size(); i++)
    {
      itk::Index<3> currentIndex = m_Subset.at(i);
      itk::Index<2> index2D;
      index2D[0] = currentIndex[0];
      index2D[1] = currentIndex[1];
      mitk::ToFProcessingCommon::ToFScalarType distance = (double)imageAcces.GetPixelByIndex(index2D);

      mitk::Point3D currentPoint;
      if (m_ReconstructionMode)
        currentPoint = mitk::ToFProcessingCommon::IndexToCartesianCoordinates(currentIndex,distance,focalLengthInPixelUnits,principalPoint);
      else
        currentPoint = mitk::ToFProcessingCommon::IndexToCartesianCoordinatesWithInterpixdist(currentIndex,distance,focalLengthInMm,m_InterPixelDistance,principalPoint);

      output->InsertPoint(i,currentPoint);
    }
  }
  else //compute PointSet holding cartesian coordinates for every image point
  {
    int xDimension = (int)input->GetDimension(0);
    int yDimension = (int)input->GetDimension(1);
    int pointCount = 0;
    mitk::ImagePixelReadAccessor<float,2> imageAcces(input, input->GetSliceData(0));
    for (int j=0; j<yDimension; j++)
    {
      for (int i=0; i<xDimension; i++)
      {
        itk::Index<2> pixel;
        pixel[0] = i;
        pixel[1] = j;

        mitk::ToFProcessingCommon::ToFScalarType distance = (double)imageAcces.GetPixelByIndex(pixel);

      mitk::Point3D currentPoint;
      if (m_ReconstructionMode)
        currentPoint = mitk::ToFProcessingCommon::IndexToCartesianCoordinates(i,j,distance,focalLengthInPixelUnits,principalPoint);
      else
        currentPoint = mitk::ToFProcessingCommon::IndexToCartesianCoordinatesWithInterpixdist(i,j,distance,focalLengthInMm,m_InterPixelDistance,principalPoint);

        if (distance>mitk::eps)
        {
          output->InsertPoint( pointCount, currentPoint );
          pointCount++;
        }
      }
    }
  }
}

void mitk::ToFDistanceImageToPointSetFilter::CreateOutputsForAllInputs()
{
  this->SetNumberOfOutputs(this->GetNumberOfInputs());  // create outputs for all inputs
  for (unsigned int idx = 0; idx < this->GetNumberOfIndexedOutputs(); ++idx)
    if (this->GetOutput(idx) == NULL)
    {
      DataObjectPointer newOutput = this->MakeOutput(idx);
      this->SetNthOutput(idx, newOutput);
    }
    this->Modified();
}

void mitk::ToFDistanceImageToPointSetFilter::GenerateOutputInformation()
{
  this->GetOutput();
  itkDebugMacro(<<"GenerateOutputInformation()");
}

void mitk::ToFDistanceImageToPointSetFilter::SetReconstructionMode(bool withoutInterpixdist)
{
  this->m_ReconstructionMode = withoutInterpixdist;
}

bool mitk::ToFDistanceImageToPointSetFilter::GetReconstructionMode()
{
  return (this->m_ReconstructionMode);
}
