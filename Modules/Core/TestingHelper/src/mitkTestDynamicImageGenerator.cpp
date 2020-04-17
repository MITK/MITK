/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkTestDynamicImageGenerator.h"
#include "mitkArbitraryTimeGeometry.h"
#include "mitkImageCast.h"
#include "mitkTemporalJoinImagesFilter.h"

namespace mitk
{
  typedef itk::Image<double, 3> FrameITKImageType;
  typedef itk::Image<double, 4> DynamicITKImageType;
  typedef itk::Image<unsigned char, 3> MaskITKImageType;

  TestImageType::Pointer GenerateTestImage(int factor)
  {
    TestImageType::Pointer image = TestImageType::New();

    TestImageType::IndexType start;
    start[0] =   0;  // first index on X
    start[1] =   0;  // first index on Y

    TestImageType::SizeType  size;
    size[0]  = 3;  // size along X
    size[1]  = 3;  // size along Y

    TestImageType::RegionType region;
    region.SetSize(size);
    region.SetIndex(start);

    image->SetRegions(region);
    image->Allocate();

    itk::ImageRegionIterator<TestImageType> it = itk::ImageRegionIterator<TestImageType>(image,
        image->GetLargestPossibleRegion());

    int count = 1;

    while (!it.IsAtEnd())
    {
      it.Set(count * factor);
      ++it;
      ++count;
    }

    return image;
  }

  TestMaskType::Pointer GenerateTestMask()
  {
    TestMaskType::Pointer image = TestMaskType::New();

    TestMaskType::IndexType start;
    start[0] =   0;  // first index on X
    start[1] =   0;  // first index on Y

    TestMaskType::SizeType  size;
    size[0]  = 3;  // size along X
    size[1]  = 3;  // size along Y

    TestMaskType::RegionType region;
    region.SetSize(size);
    region.SetIndex(start);

    image->SetRegions(region);
    image->Allocate();

    itk::ImageRegionIterator<TestMaskType> it = itk::ImageRegionIterator<TestMaskType>(image,
        image->GetLargestPossibleRegion());

    int count = 1;

    while (!it.IsAtEnd())
    {
      if (count > 1 && count < 5)
      {
        it.Set(1);
      }
      else
      {
        it.Set(0);
      }

      ++it;
      ++count;
    }

    return image;
  }

  Image::Pointer GenerateTestFrame(double timePoint)
  {
    FrameITKImageType::Pointer image = FrameITKImageType::New();

    FrameITKImageType::IndexType start;
    start[0] =   0;  // first index on X
    start[1] =   0;  // first index on Y
    start[2] =   0;  // first index on Z

    FrameITKImageType::SizeType  size;
    size[0]  = 3;  // size along X
    size[1]  = 3;  // size along Y
    size[2]  = 3;  // size along Z

    FrameITKImageType::RegionType region;
    region.SetSize(size);
    region.SetIndex(start);

    image->SetRegions(region);
    image->Allocate();

    itk::ImageRegionIterator<FrameITKImageType> it = itk::ImageRegionIterator<FrameITKImageType>(image,
        image->GetLargestPossibleRegion());

    int count = 0;

    while (!it.IsAtEnd())
    {
      double slope = count % (size[0] * size[1]);
      double offset = itk::Math::Floor<double, double>(count / (size[0] * size[1])) * 10;

      it.Set(slope * timePoint + offset);
      ++it;
      ++count;
    }

    mitk::Image::Pointer mitkImage = mitk::Image::New();
    mitkImage->InitializeByItk(image.GetPointer());
    mitkImage->SetVolume(image->GetBufferPointer());

    return mitkImage;
  }

  Image::Pointer GenerateTestMaskMITK()
  {
    MaskITKImageType::Pointer image = MaskITKImageType::New();

    MaskITKImageType::IndexType start;
    start[0] =   0;  // first index on X
    start[1] =   0;  // first index on Y
    start[2] =   0;  // first index on Z

    MaskITKImageType::SizeType  size;
    size[0]  = 3;  // size along X
    size[1]  = 3;  // size along Y
    size[2]  = 3;  // size along Z

    MaskITKImageType::RegionType region;
    region.SetSize(size);
    region.SetIndex(start);

    image->SetRegions(region);
    image->Allocate();

    itk::ImageRegionIterator<MaskITKImageType> it = itk::ImageRegionIterator<MaskITKImageType>(image,
        image->GetLargestPossibleRegion());

    int count = 0;

    while (!it.IsAtEnd())
    {
      if (count < 14)
      {
        it.Set(1);
      }
      else
      {
        it.Set(0);
      }

      ++it;
      ++count;
    }

    mitk::Image::Pointer mitkImage = mitk::Image::New();
    mitkImage->InitializeByItk(image.GetPointer());
    mitkImage->SetVolume(image->GetBufferPointer());

    return mitkImage;
  }


  Image::Pointer GenerateDynamicTestImageMITK()
  {
    auto filter = mitk::TemporalJoinImagesFilter::New();

    mitk::TemporalJoinImagesFilter::TimeBoundsVectorType bounds;
    for (int i = 0; i < 10; ++i)
    {
      filter->SetInput(i, GenerateTestFrame(1 + (5.0 * i)));
      bounds.push_back(1 + (5.0 * (i + 1)));
    }

    filter->SetFirstMinTimeBound(1.);
    filter->SetMaxTimeBounds(bounds);

    filter->Update();

    return filter->GetOutput();
  }

}

