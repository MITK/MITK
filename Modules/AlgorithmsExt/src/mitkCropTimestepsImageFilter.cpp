/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkCropTimestepsImageFilter.h"

#include <mitkImage.h>
#include <mitkArbitraryTimeGeometry.h>
#include <mitkImageTimeSelector.h>
#include <mitkImageReadAccessor.h>


  void mitk::CropTimestepsImageFilter::VerifyInputImage(const mitk::Image* inputImage) const
  {
    if (!inputImage->IsInitialized())
      mitkThrow() << "Input image is not initialized.";

    if (!inputImage->IsVolumeSet())
      mitkThrow() << "Input image volume is not set.";

    auto geometry = inputImage->GetGeometry();

    if (nullptr == geometry || !geometry->IsValid())
      mitkThrow() << "Input image has invalid geometry.";

    if (inputImage->GetDimension() != 4) {
      mitkThrow() << "CropTimestepsImageFilter only works with 2D+t and 3D+t images.";
    }

    if (inputImage->GetTimeSteps() ==1) {
      mitkThrow() << "Input image has only one timestep.";
    }

    if (!geometry->GetImageGeometry())
      mitkThrow() << "Geometry of input image is not an image geometry.";
  }

  void mitk::CropTimestepsImageFilter::GenerateOutputInformation()
  {
    Image::ConstPointer input = this->GetInput();
    Image::Pointer output = this->GetOutput();
    if (m_LowerBoundaryTimestep > m_UpperBoundaryTimestep) {
      mitkThrow() << "lower timestep is larger than upper timestep.";
    }
    if (m_UpperBoundaryTimestep == std::numeric_limits<unsigned int>::max()) {
      m_UpperBoundaryTimestep = input->GetTimeSteps();
    }
    else if (m_UpperBoundaryTimestep > input->GetTimeSteps()) {
      m_UpperBoundaryTimestep = input->GetTimeSteps();
      MITK_WARN << "upper boundary timestep set to " << m_UpperBoundaryTimestep;
    }
    m_DesiredRegion = ComputeDesiredRegion();
    unsigned int dimension = input->GetDimension();
    auto dimensions = new unsigned int[dimension];
    itk2vtk(m_DesiredRegion.GetSize(), dimensions);
    if (dimension > 3)
      memcpy(dimensions + 3, input->GetDimensions() + 3, (dimension - 3) * sizeof(unsigned int));

    dimensions[3] = m_UpperBoundaryTimestep - m_LowerBoundaryTimestep;

    // create basic slicedGeometry that will be initialized below
    output->Initialize(mitk::PixelType(input->GetPixelType()), dimension, dimensions);
    delete[] dimensions;
    auto newTimeGeometry = AdaptTimeGeometry(input->GetTimeGeometry(), m_LowerBoundaryTimestep, m_UpperBoundaryTimestep);
    output->SetTimeGeometry(newTimeGeometry);
    output->SetPropertyList(input->GetPropertyList());
  }

  mitk::SlicedData::RegionType mitk::CropTimestepsImageFilter::ComputeDesiredRegion() const
  {
    auto desiredRegion = this->GetInput()->GetLargestPossibleRegion();
    auto index = desiredRegion.GetIndex();
    auto size = desiredRegion.GetSize();
    unsigned int timeDimension = 3;
    index[timeDimension] = m_LowerBoundaryTimestep;
    size[timeDimension] = m_UpperBoundaryTimestep - m_LowerBoundaryTimestep;
    desiredRegion.SetIndex(index);
    desiredRegion.SetSize(size);
    return desiredRegion;
  }

  mitk::TimeGeometry::Pointer mitk::CropTimestepsImageFilter::AdaptTimeGeometry(mitk::TimeGeometry::ConstPointer sourceGeometry, unsigned int startTimestep, unsigned int endTimestep) const
  {
    auto newTimeGeometry = mitk::ArbitraryTimeGeometry::New();
    newTimeGeometry->ClearAllGeometries();
    for (unsigned int timestep = startTimestep; timestep < endTimestep; timestep++)
    {
      auto geometryForTimePoint = sourceGeometry->GetGeometryForTimeStep(timestep);
      auto minTP = sourceGeometry->GetMinimumTimePoint(timestep);
      auto maxTP = sourceGeometry->GetMaximumTimePoint(timestep);
      ///////////////////////////////////////
      // Workarround T27883. See https://phabricator.mitk.org/T27883#219473 for more details.
      // This workarround should be removed as soon as T28262 is solved!
      if (timestep + 1 == sourceGeometry->CountTimeSteps() && minTP == maxTP)
      {
        maxTP = minTP + 1.;
      }
      // End of workarround for T27883
      //////////////////////////////////////

      newTimeGeometry->AppendNewTimeStepClone(geometryForTimePoint, minTP, maxTP);
    }
    return newTimeGeometry.GetPointer();
  }

void mitk::CropTimestepsImageFilter::GenerateData()
{
  const auto* inputImage = this->GetInput();
  mitk::Image::Pointer output = this->GetOutput();

  if ((output->IsInitialized() == false))
    return;

  auto timeSelector = mitk::ImageTimeSelector::New();

  timeSelector->SetInput(inputImage);

  unsigned int timeStart = m_DesiredRegion.GetIndex(3);
  unsigned int timeEnd = timeStart + m_DesiredRegion.GetSize(3);
  for (unsigned int timestep = timeStart; timestep < timeEnd; ++timestep)
  {
    timeSelector->SetTimeNr(timestep);
    timeSelector->UpdateLargestPossibleRegion();
    mitk::ImageReadAccessor imageAccessorWithOneTimestep(timeSelector->GetOutput());
    output->SetVolume(imageAccessorWithOneTimestep.GetData(), timestep-timeStart);
  }
}

void mitk::CropTimestepsImageFilter::SetInput(const InputImageType* image)
{
  if (this->GetInput() == image)
    return;

  Superclass::SetInput(image);
}

void mitk::CropTimestepsImageFilter::SetInput(unsigned int index, const InputImageType* image)
{
  if (0 != index)
    mitkThrow() << "Input index " << index << " is invalid.";

  this->SetInput(image);
}

void mitk::CropTimestepsImageFilter::VerifyInputInformation()
{
  Superclass::VerifyInputInformation();

  VerifyInputImage(this->GetInput());
}
