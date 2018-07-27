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

#include "mitkTestArtifactGenerator.h"
#include "mitkArbitraryTimeGeometry.h"
#include "mitkImageCast.h"

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

    mitk::Image::Pointer tempImage = GenerateTestFrame(1);
    mitk::Image::Pointer dynamicImage = mitk::Image::New();

    DynamicITKImageType::Pointer dynamicITKImage = DynamicITKImageType::New();
    DynamicITKImageType::RegionType dynamicITKRegion;
    DynamicITKImageType::PointType dynamicITKOrigin;
    DynamicITKImageType::IndexType dynamicITKIndex;
    DynamicITKImageType::SpacingType dynamicITKSpacing;

    dynamicITKSpacing[0] = tempImage->GetGeometry()->GetSpacing()[0];
    dynamicITKSpacing[1] = tempImage->GetGeometry()->GetSpacing()[1];
    dynamicITKSpacing[2] = tempImage->GetGeometry()->GetSpacing()[2];
    dynamicITKSpacing[3] = 5.0;

    dynamicITKIndex[0] = 0;  // The first pixel of the REGION
    dynamicITKIndex[1] = 0;
    dynamicITKIndex[2] = 0;
    dynamicITKIndex[3] = 0;

    dynamicITKRegion.SetSize(0, tempImage->GetDimension(0));
    dynamicITKRegion.SetSize(1, tempImage->GetDimension(1));
    dynamicITKRegion.SetSize(2, tempImage->GetDimension(2));
    dynamicITKRegion.SetSize(3, 10);

    dynamicITKRegion.SetIndex(dynamicITKIndex);

    dynamicITKOrigin[0] = tempImage->GetGeometry()->GetOrigin()[0];
    dynamicITKOrigin[1] = tempImage->GetGeometry()->GetOrigin()[1];
    dynamicITKOrigin[2] = tempImage->GetGeometry()->GetOrigin()[2];

    dynamicITKImage->SetOrigin(dynamicITKOrigin);
    dynamicITKImage->SetSpacing(dynamicITKSpacing);
    dynamicITKImage->SetRegions(dynamicITKRegion);
    dynamicITKImage->Allocate();
    dynamicITKImage->FillBuffer(0); //not sure if this is necessary

    // Convert
    mitk::CastToMitkImage(dynamicITKImage, dynamicImage);

    ArbitraryTimeGeometry::Pointer timeGeometry = ArbitraryTimeGeometry::New();
    timeGeometry->ClearAllGeometries();


    for (int i = 0; i < 10; ++i)
    {
      mitk::Image::Pointer frameImage = GenerateTestFrame(1 + (dynamicITKSpacing[3] * i));
      mitk::ImageReadAccessor accessor(frameImage);
      dynamicImage->SetVolume(accessor.GetData(), i);

      timeGeometry->AppendNewTimeStepClone(frameImage->GetGeometry(), 1 + (dynamicITKSpacing[3] * i),
                                        1 + (dynamicITKSpacing[3]*(i+1)));
    }

    dynamicImage->SetTimeGeometry(timeGeometry);

    return dynamicImage;
  }

}

