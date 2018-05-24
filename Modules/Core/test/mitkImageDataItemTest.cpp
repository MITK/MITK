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

#include <array>

#include "mitkTestFixture.h"
#include "mitkTestingMacros.h"

#include <mitkImageDataItem.h>

#include <mitkPixelType.h>
#include <mitkImage.h>
#include <mitkImagePixelWriteAccessor.h>

class mitkImageDataItemTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkImageDataItemTestSuite);
  MITK_TEST(TestAccessOnHugeImage);
  CPPUNIT_TEST_SUITE_END();

private:
  mitk::Image::Pointer m_Image;

public:
  void setUp() override
  {
    m_Image = mitk::Image::New();
    mitk::PixelType pixelType = mitk::MakeScalarPixelType<unsigned char>();

    std::array<unsigned int, 3> dimensions = {{ 1700, 1700, 1700 }};
    m_Image->Initialize(pixelType, 3, dimensions.data());
  }

  void TestAccessOnHugeImage()
  {
    CPPUNIT_ASSERT(m_Image.IsNotNull());

    try
    {
      mitk::ImagePixelWriteAccessor<unsigned char, 3> writeAccess(m_Image.GetPointer(), m_Image->GetVolumeData());

      auto* voxelStart = writeAccess.GetData();
      size_t imageSize = 1;
      for (unsigned int i = 0; i < m_Image->GetDimension(); i++)
        imageSize *= m_Image->GetDimension(i);

      auto* voxelEnd = voxelStart + imageSize;

      CPPUNIT_ASSERT(writeAccess.GetData() != nullptr);
      auto* accessCheck = voxelEnd - 1;
      *accessCheck = 1;
    }
    catch (const itk::MemoryAllocationError& e)
    {
      MITK_ERROR << e.what();
      exit(77);
    }
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkImageDataItem)
