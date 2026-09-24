/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkCompressedImageContainer.h>

#include <mitkImageReadAccessor.h>
#include <mitkImageWriteAccessor.h>

#include <itkMultiThreaderBase.h>

#include <lz4.h>

#include <algorithm>

mitk::CompressedImageContainer::CompressedImageContainer()
  : m_Dimension(0)
{
}

mitk::CompressedImageContainer::~CompressedImageContainer()
{
  this->ClearCompressedImageData();
}

void mitk::CompressedImageContainer::ClearCompressedImageData()
{
  for (const auto& image : m_CompressedImageData)
  {
    for (auto slice : image)
      delete[] slice.second;
  }

  m_CompressedImageData.clear();

  m_PixelType = nullptr;
  m_TimeGeometry = nullptr;
  m_SliceDimensions[0] = 0;
  m_SliceDimensions[1] = 0;
  m_Dimension = 0;
}

void mitk::CompressedImageContainer::CompressImage(const Image* image)
{
  this->ClearCompressedImageData();

  if (nullptr == image)
    return;

  m_PixelType = std::make_unique<PixelType>(image->GetPixelType());
  m_TimeGeometry = image->GetTimeGeometry()->Clone();
  m_SliceDimensions[0] = image->GetDimension(0);
  m_SliceDimensions[1] = image->GetDimension(1);
  m_Dimension = image->GetDimension();

  const auto numTimeSteps = m_TimeGeometry->CountTimeSteps();
  const auto numSlices = image->GetDimension(2);
  const auto numSliceBytes = image->GetPixelType().GetSize() * image->GetDimension(0) * image->GetDimension(1);

  m_CompressedImageData.resize(numTimeSteps);

  auto threader = itk::MultiThreaderBase::New();

  for (std::remove_const_t<decltype(numTimeSteps)> t = 0; t < numTimeSteps; ++t)
  {
    auto& slices = m_CompressedImageData[t];
    slices.resize(numSlices, { 0, nullptr });

    ImageReadAccessor accessor(image, image->GetVolumeData(t));
    const auto* volume = reinterpret_cast<const char*>(accessor.GetData());

    threader->ParallelizeArray(0, numSlices, [&](itk::SizeValueType s)
      {
        const auto* src = volume + numSliceBytes * s;
        char* dest = new char[numSliceBytes];
        const auto destSize = LZ4_compress_default(src, dest, static_cast<int>(numSliceBytes), static_cast<int>(numSliceBytes));

        if (0 == destSize)
        {
          MITK_ERROR << "LZ4 compression failed!";
          delete[] dest;
        }
        else
        {
          char* shrinkedDest = new char[destSize];
          std::copy(dest, dest + destSize, shrinkedDest);
          delete[] dest;
          slices[s] = { destSize, shrinkedDest };
        }
      }, nullptr);
  }
}

mitk::Image::Pointer mitk::CompressedImageContainer::DecompressImage() const
{
  if (m_CompressedImageData.empty())
    return nullptr;

  const auto numSlices = static_cast<unsigned int>(m_CompressedImageData[0].size());
  const auto numTimeSteps = static_cast<unsigned int>(m_CompressedImageData.size());
  const auto numSliceBytes = m_PixelType->GetSize() * m_SliceDimensions[0] * m_SliceDimensions[1];

  std::array<unsigned int, 4> dimensions;
  dimensions[0] = m_SliceDimensions[0];
  dimensions[1] = m_SliceDimensions[1];
  dimensions[2] = numSlices;
  dimensions[3] = numTimeSteps;

  auto image = Image::New();
  image->Initialize(*m_PixelType, m_Dimension, dimensions.data());

  auto threader = itk::MultiThreaderBase::New();

  for (std::remove_const_t<decltype(numTimeSteps)> t = 0; t < numTimeSteps; ++t)
  {
    ImageWriteAccessor accessor(image, image->GetVolumeData(static_cast<int>(t)));
    auto* volume = reinterpret_cast<char*>(accessor.GetData());

    threader->ParallelizeArray(0, numSlices, [&](itk::SizeValueType s)
      {
        auto* dest = volume + numSliceBytes * s;
        const auto& slice = m_CompressedImageData[t][s];
        const auto destSize = LZ4_decompress_safe(slice.second, dest, slice.first, static_cast<int>(numSliceBytes));

        if (0 > destSize)
          MITK_ERROR << "LZ4 decompression failed!";
      }, nullptr);
  }

  image->SetTimeGeometry(m_TimeGeometry->Clone());

  return image;
}
