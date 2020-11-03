/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkTemporalJoinImagesFilter.h"

#include <numeric>

#include "mitkArbitraryTimeGeometry.h"
#include "mitkImageReadAccessor.h"
#include "mitkTemporoSpatialStringProperty.h"

void mitk::TemporalJoinImagesFilter::SetMaxTimeBounds(const TimeBoundsVectorType& timeBounds)
{
  m_MaxTimeBounds = timeBounds;
  this->Modified();
}

void mitk::TemporalJoinImagesFilter::GenerateInputRequestedRegion()
{
  Superclass::GenerateInputRequestedRegion();
  const auto nrOfInputs = this->GetNumberOfInputs();
  for (DataObjectPointerArraySizeType pos = 0; pos < nrOfInputs; ++pos)
  {
    this->GetInput(pos)->SetRequestedRegionToLargestPossibleRegion();
  }
}

void mitk::TemporalJoinImagesFilter::GenerateOutputInformation()
{
  mitk::Image::ConstPointer input = this->GetInput();
  mitk::Image::Pointer output = this->GetOutput();

  const auto nrOfInputs = this->GetNumberOfInputs();
  auto timeBounds = m_MaxTimeBounds;

  if (timeBounds.empty())
  {
    timeBounds.resize(nrOfInputs);
    std::iota(timeBounds.begin(), timeBounds.end(), 1.0);
  }
  else if(timeBounds.size() != nrOfInputs)
  {
    mitkThrow() << "User defined max time bounds do not match the number if inputs (" << nrOfInputs << "). Size of max timebounds is " << timeBounds.size() << ", but it should be " << nrOfInputs << ".";
  }

  timeBounds.insert(timeBounds.begin(), m_FirstMinTimeBound);

  auto timeGeo = mitk::ArbitraryTimeGeometry::New();
  timeGeo->ReserveSpaceForGeometries(nrOfInputs);

  for (DataObjectPointerArraySizeType pos = 0; pos < nrOfInputs; ++pos)
  {
    timeGeo->AppendNewTimeStepClone(this->GetInput(pos)->GetGeometry(), timeBounds[pos], timeBounds[pos + 1]);
  }
  output->Initialize(input->GetPixelType(), *timeGeo);

  auto newPropList = input->GetPropertyList()->Clone();
  for (DataObjectPointerArraySizeType pos = 1; pos < nrOfInputs; ++pos)
  {
    const auto otherList = this->GetInput(pos)->GetPropertyList();
    for (const auto& key : otherList->GetPropertyKeys())
    {
      auto prop = newPropList->GetProperty(key);
      if (prop == nullptr)
      {
        newPropList->SetProperty(key, otherList->GetProperty(key)->Clone());
      }
      else
      {
        auto tempoSpatialProp = dynamic_cast<mitk::TemporoSpatialStringProperty*>(prop);
        auto oTempoSpatialProp = dynamic_cast<mitk::TemporoSpatialStringProperty*>(otherList->GetProperty(key));
        if (prop != nullptr && oTempoSpatialProp != nullptr)
        {
          auto availabelSlices = oTempoSpatialProp->GetAvailableSlices(0);

          for (const auto& sliceID : availabelSlices)
          {
            tempoSpatialProp->SetValue(pos, sliceID, oTempoSpatialProp->GetValueBySlice(sliceID));
          }
        }
        //other prop types can be ignored, we only use the values of the first frame.
      }
    }
  }

  output->SetPropertyList(newPropList);
}


void mitk::TemporalJoinImagesFilter::GenerateData()
{
  mitk::Image::Pointer output = this->GetOutput();
  mitk::Image::ConstPointer refInput = this->GetInput();
  const auto nrOfInputs = this->GetNumberOfInputs();

  for (DataObjectPointerArraySizeType pos = 0; pos < nrOfInputs; ++pos)
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

  for (DataObjectPointerArraySizeType pos = 0; pos < nrOfInputs; ++pos)
  {
    mitk::ImageReadAccessor accessor(this->GetInput(pos));
    output->SetVolume(accessor.GetData(), pos);
  }
}
