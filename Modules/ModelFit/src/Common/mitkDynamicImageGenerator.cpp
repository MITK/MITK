/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkDynamicImageGenerator.h"

#include <numeric>

#include "mitkArbitraryTimeGeometry.h"
#include "mitkImageReadAccessor.h"

void mitk::DynamicImageGenerationFilter::SetTimeBounds(const TimeBoundsVectorType& timeBounds)
{
  m_TimeBounds = timeBounds;
  this->Modified();
}

void mitk::DynamicImageGenerationFilter::GenerateInputRequestedRegion()
{
  Superclass::GenerateInputRequestedRegion();

  mitk::Image* input = this->GetInput();

  input->SetRequestedRegionToLargestPossibleRegion();
}

void mitk::DynamicImageGenerationFilter::GenerateOutputInformation()
{
  mitk::Image::ConstPointer input = this->GetInput();
  mitk::Image::Pointer output = this->GetOutput();

  auto timeBounds = m_TimeBounds;

  if (timeBounds.empty())
  {
    timeBounds.resize(this->GetNumberOfInputs() + 1);
    std::iota(timeBounds.begin(), timeBounds.end(), 0.0);
  }
  else if(timeBounds.size() != this->GetNumberOfInputs() + 1)
  {
    mitkThrow() << "User defined time bounds does not match the number if inputs (" << this->GetNumberOfInputs() << "). Size of timebounds is " << timeBounds.size() << ", but it should be " << this->GetNumberOfInputs() + 1 << ".";
  }

  auto timeGeo = mitk::ArbitraryTimeGeometry::New();
  timeGeo->ReserveSpaceForGeometries(this->GetNumberOfInputs());

  for (DataObjectPointerArraySizeType pos = 0; pos < this->GetNumberOfInputs(); pos++)
  {
    timeGeo->AppendNewTimeStepClone(this->GetInput(pos)->GetGeometry(), timeBounds[pos], timeBounds[pos + 1]);
  }
  
  output->Initialize(input->GetPixelType(), *timeGeo);
  output->SetPropertyList(input->GetPropertyList()->Clone());
}


void mitk::DynamicImageGenerationFilter::GenerateData()
{
  mitk::Image::Pointer output = this->GetOutput();
  mitk::Image::ConstPointer refInput = this->GetInput();

  for (DataObjectPointerArraySizeType pos = 0; pos < this->GetNumberOfInputs(); pos++)
  {
    if (!Equal(*(refInput->GetGeometry()), *(this->GetInput(pos)->GetGeometry()), mitk::eps, false))
    {
      mitkThrow() << "Cannot fuse images. At least image #" << pos << " has another geometry than the first image.";
    }
    if (refInput->GetPixelType() != this->GetInput(pos)->GetPixelType())
    {
      mitkThrow() << "Cannot fuse images. At least image #" << pos << " has another pixeltype than the first image.";
    }
  }

  for (DataObjectPointerArraySizeType pos = 0; pos < this->GetNumberOfInputs(); pos++)
  {
    mitk::ImageReadAccessor accessor(this->GetInput(pos));
    output->SetVolume(accessor.GetData(), pos);
  }
}
