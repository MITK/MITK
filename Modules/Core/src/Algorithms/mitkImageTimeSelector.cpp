/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkImageTimeSelector.h"

mitk::ImageTimeSelector::ImageTimeSelector() : m_TimeNr(0), m_ChannelNr(0)
{
}

mitk::ImageTimeSelector::~ImageTimeSelector()
{
}

void mitk::ImageTimeSelector::GenerateOutputInformation()
{
  Image::ConstPointer input = this->GetInput();
  Image::Pointer output = this->GetOutput();

  itkDebugMacro(<< "GenerateOutputInformation()");

  int dim = (input->GetDimension() < 3 ? input->GetDimension() : 3);
  output->Initialize(input->GetPixelType(), dim, input->GetDimensions());

  if ((unsigned int)m_TimeNr >= input->GetDimension(3))
  {
    m_TimeNr = input->GetDimension(3) - 1;
  }

  // initialize geometry
  mitk::SlicedGeometry3D::Pointer sliced_geo = input->GetSlicedGeometry(m_TimeNr);
  if (sliced_geo.IsNull())
  {
    mitkThrow() << "Failed to retrieve SlicedGeometry from input at timestep " << m_TimeNr;
  }

  mitk::SlicedGeometry3D::Pointer sliced_geo_clone = sliced_geo->Clone();
  if (sliced_geo_clone.IsNull())
  {
    mitkThrow() << "Failed to clone the retrieved sliced geometry.";
  }

  mitk::BaseGeometry::Pointer geom_3d = dynamic_cast<BaseGeometry *>(sliced_geo_clone.GetPointer());
  if (geom_3d.IsNotNull())
  {
    output->SetGeometry(geom_3d.GetPointer());
  }
  else
  {
    mitkThrow() << "Failed to cast the retrieved SlicedGeometry to a Geometry3D object.";
  }

  output->SetPropertyList(input->GetPropertyList()->Clone());
}

void mitk::ImageTimeSelector::GenerateData()
{
  const Image::RegionType &requestedRegion = this->GetOutput()->GetRequestedRegion();

  // do we really need a complete volume at a time?
  if (requestedRegion.GetSize(2) > 1)
  {
    mitk::ImageDataItem::Pointer im = this->GetVolumeData(m_TimeNr, m_ChannelNr)->Clone();
    im->SetTimestep(0);
    im->SetManageMemory(false);
    this->SetVolumeItem(im, 0);
  }
  else
    // no, so take just a slice!
    this->SetSliceItem(
      this->GetSliceData(requestedRegion.GetIndex(2), m_TimeNr, m_ChannelNr), requestedRegion.GetIndex(2), 0);
}

void mitk::ImageTimeSelector::GenerateInputRequestedRegion()
{
  Superclass::GenerateInputRequestedRegion();

  ImageToImageFilter::InputImagePointer input = this->GetInput();
  Image::Pointer output = this->GetOutput();

  Image::RegionType requestedRegion;
  requestedRegion = output->GetRequestedRegion();
  requestedRegion.SetIndex(3, m_TimeNr);
  requestedRegion.SetIndex(4, m_ChannelNr);
  requestedRegion.SetSize(3, 1);
  requestedRegion.SetSize(4, 1);

  input->SetRequestedRegion(&requestedRegion);
}

mitk::Image::ConstPointer mitk::SelectImageByTimeStep(const mitk::Image* image, unsigned int timestep)
{
  if (nullptr == image)
    return image;

  if (image->GetDimension() != 4)
    return image;

  ImageTimeSelector::Pointer imageTimeSelector = mitk::ImageTimeSelector::New();

  imageTimeSelector->SetInput(image);
  imageTimeSelector->SetTimeNr(static_cast<int>(timestep));

  imageTimeSelector->UpdateLargestPossibleRegion();

  return imageTimeSelector->GetOutput();
}

mitk::Image::Pointer mitk::SelectImageByTimeStep(mitk::Image* image, unsigned int timestep)
{
  if (nullptr == image)
    return image;

  if (image->GetDimension() != 4)
    return image;

  ImageTimeSelector::Pointer imageTimeSelector = mitk::ImageTimeSelector::New();

  imageTimeSelector->SetInput(image);
  imageTimeSelector->SetTimeNr(static_cast<int>(timestep));

  imageTimeSelector->UpdateLargestPossibleRegion();

  return imageTimeSelector->GetOutput();
}

mitk::Image::ConstPointer mitk::SelectImageByTimePoint(const mitk::Image* image, TimePointType timePoint)
{
  if (nullptr == image)
    return image;

  if (!image->GetTimeGeometry()->IsValidTimePoint(timePoint))
    return nullptr;

  return SelectImageByTimeStep(image, image->GetTimeGeometry()->TimePointToTimeStep(timePoint));
}

mitk::Image::Pointer mitk::SelectImageByTimePoint(mitk::Image* image, TimePointType timePoint)
{
  if (nullptr == image)
    return image;

  if (!image->GetTimeGeometry()->IsValidTimePoint(timePoint))
    return nullptr;

  return SelectImageByTimeStep(image, image->GetTimeGeometry()->TimePointToTimeStep(timePoint));
}
