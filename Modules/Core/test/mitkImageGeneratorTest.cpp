/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

// Testing
#include "mitkTestFixture.h"
#include <mitkTestingMacros.h>
// MITK includes
#include <mitkCoreServices.h>
#include "mitkImage.h"
#include "mitkImageGenerator.h"
#include "mitkImageReadAccessor.h"
#include "mitkImageStatisticsHolder.h"

class mitkImageGeneratorTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkImageGeneratorTestSuite);
  MITK_TEST(SetSpacingX2D_Success);
  MITK_TEST(SetDefaultSpacingX2D_Success);
  MITK_TEST(SetSpacingX3D_Success);
  MITK_TEST(SetSpacingY2D_Success);
  MITK_TEST(SetDefaultSpacingY2D_Success);
  MITK_TEST(SetSpacingY3D_Success);
  MITK_TEST(SetSpacingZ2D_Success);
  MITK_TEST(SetDefaultSpacingZ2D_Success);
  MITK_TEST(SetSpacingZ3D_Success);
  MITK_TEST(SetDimension2D_Success);
  MITK_TEST(SetDimension3D_Success);
  MITK_TEST(SetDimension4D_Success);
  MITK_TEST(SetDimensionX2D_Success);
  MITK_TEST(SetDimensionY2D_Success);
  MITK_TEST(SetDimensionZ3D_Success);
  MITK_TEST(SetDimensionT4D_Success);
  MITK_TEST(SetDimensions3Dc_Success);
  MITK_TEST(SetDataTypeFloat2D_Success);
  MITK_TEST(SetDataTypeUChar2D_Success);
  MITK_TEST(SetDataTypeInt3D_Success);
  MITK_TEST(SetDataTypeDouble3D_Success);
  MITK_TEST(SetDataTypeFloat4D_Success);
  MITK_TEST(SetDataTypeUChar4D_Success);
  MITK_TEST(SetDataTypeUInt3D_Success);
  MITK_TEST(SetPixelTypeFloat2D_Success);
  MITK_TEST(SetPixelTypeUChar2D_Success);
  MITK_TEST(SetPixelTypeInt3D_Success);
  MITK_TEST(SetPixelTypeDouble3D_Success);
  MITK_TEST(SetPixelTypeFloat4D_Success);
  MITK_TEST(SetPixelTypeUChar4D_Success);
  MITK_TEST(SetPixelTypeUInt3D_Success);
  MITK_TEST(MaxValueHolds_Success);
  MITK_TEST(MinValueHolds_Success);
  MITK_TEST(DefaultMaxValueHolds_Success);
  MITK_TEST(DefaultMinValueHolds_Success);
  MITK_TEST(SetGradientImageValues_Success);
  CPPUNIT_TEST_SUITE_END();

private:
  // create some images with arbitrary parameters (corner cases)
  mitk::Image::Pointer m_Image2Da;
  mitk::Image::Pointer m_Image2Db;
  mitk::Image::Pointer m_Image3Da;
  mitk::Image::Pointer m_Image3Db;
  mitk::Image::Pointer m_Image4Da;
  mitk::Image::Pointer m_Image4Db;
  mitk::Image::Pointer m_Image3Dc;

public:
  void setUp()
  {
    m_Image2Da = mitk::ImageGenerator::GenerateRandomImage<float>(2, 4, 0, 0, 0.1, 0.2, 0.3, 577, 23);
    m_Image2Db = mitk::ImageGenerator::GenerateRandomImage<unsigned char>(1, 1, 0, 0);
    m_Image3Da = mitk::ImageGenerator::GenerateRandomImage<int>(2, 4, 1, 0);
    m_Image3Db = mitk::ImageGenerator::GenerateRandomImage<double>(2, 4, 8, 0);
    m_Image4Da = mitk::ImageGenerator::GenerateRandomImage<float>(2, 4, 8, 1);
    m_Image4Db = mitk::ImageGenerator::GenerateRandomImage<unsigned char>(2, 4, 8, 2);
    m_Image3Dc = mitk::ImageGenerator::GenerateGradientImage<unsigned int>(1, 2, 3, 4, 5, 6);
  }
  void tearDown()
  {
    m_Image2Da = nullptr;
    m_Image2Db = nullptr;
    m_Image3Da = nullptr;
    m_Image3Db = nullptr;
    m_Image4Da = nullptr;
    m_Image4Db = nullptr;
    m_Image3Dc = nullptr;
  }

  void SetSpacingX2D_Success()
  {
    CPPUNIT_ASSERT_MESSAGE("Testing if spacing 2D x is set correctly.",
        fabs(m_Image2Da->GetGeometry()->GetSpacing()[0] - 0.1) < 0.0001);
  }

  void SetDefaultSpacingX2D_Success()
  {
    CPPUNIT_ASSERT_MESSAGE("Testing if default spacing 2D x is set correctly.",
                fabs(m_Image2Db->GetGeometry()->GetSpacing()[0] - 1.0) < 0.0001);
  }

  void SetSpacingX3D_Success()
  {
    CPPUNIT_ASSERT_MESSAGE("Testing if spacing 3D x is set correctly.",
          fabs(m_Image3Dc->GetGeometry()->GetSpacing()[0] - 4) < 0.0001);
  }

  void SetSpacingY2D_Success()
  {
    CPPUNIT_ASSERT_MESSAGE("Testing if spacing 2D y is set correctly.",
        fabs(m_Image2Da->GetGeometry()->GetSpacing()[1] - 0.2) < 0.0001);
  }

  void SetDefaultSpacingY2D_Success()
  {
    CPPUNIT_ASSERT_MESSAGE("Testing if default spacing 2D y is set correctly.",
                 fabs(m_Image2Db->GetGeometry()->GetSpacing()[1] - 1.0) < 0.0001);
  }

  void SetSpacingY3D_Success()
  {
    CPPUNIT_ASSERT_MESSAGE("Testing if spacing 3D y is set correctly.",
          fabs(m_Image3Dc->GetGeometry()->GetSpacing()[1] - 5) < 0.0001);
  }

  void SetSpacingZ2D_Success()
  {
    CPPUNIT_ASSERT_MESSAGE("Testing if spacing 2D z is set correctly.",
        fabs(m_Image2Da->GetGeometry()->GetSpacing()[2] - 0.3) < 0.0001);
  }

  void SetDefaultSpacingZ2D_Success()
  {
    CPPUNIT_ASSERT_MESSAGE("Testing if default spacing 2D z is set correctly.",
                 fabs(m_Image2Db->GetGeometry()->GetSpacing()[2] - 1.0) < 0.0001);
  }

  void SetSpacingZ3D_Success()
  {
    CPPUNIT_ASSERT_MESSAGE("Testing if spacing z is set correctly.",
        fabs(m_Image3Dc->GetGeometry()->GetSpacing()[2] - 6) < 0.0001);
  }

  void SetDimension2D_Success()
  {
    CPPUNIT_ASSERT_MESSAGE("Testing if the dimension 2D is set correctly.", m_Image2Da->GetDimension() == 2);
    CPPUNIT_ASSERT_MESSAGE("Testing if the dimension 2D is set correctly.", m_Image2Db->GetDimension() == 2);
  }

  void SetDimension3D_Success()
  {
    CPPUNIT_ASSERT_MESSAGE("Testing if the dimension 3D is set correctly.",m_Image3Da->GetDimension() == 2);
    CPPUNIT_ASSERT_MESSAGE("Testing if the dimension 3D is set correctly.", m_Image3Db->GetDimension() == 3);
  }

  void SetDimension4D_Success()
  {
    CPPUNIT_ASSERT_MESSAGE("Testing if the dimension 4D is set correctly.", m_Image4Da->GetDimension() == 3);
    CPPUNIT_ASSERT_MESSAGE("Testing if the dimension 4D is set correctly.", m_Image4Db->GetDimension() == 4);
  }

  void SetDimensionX2D_Success()
  {
    CPPUNIT_ASSERT_MESSAGE("Testing if the X dimension of the 2D image is set correctly.", m_Image2Da->GetDimension(0) == 2);
  }

  void SetDimensionY2D_Success()
  {
    CPPUNIT_ASSERT_MESSAGE("Testing if the Y dimension of the 2D image is set correctly.", m_Image2Db->GetDimension(1) == 1);
  }

  void SetDimensionZ3D_Success()
  {
    CPPUNIT_ASSERT_MESSAGE("Testing if the Z dimension of the 3D image is set correctly.", m_Image3Da->GetDimension(2) == 1);
    CPPUNIT_ASSERT_MESSAGE("Testing if the Z dimension of the 3D image is set correctly.", m_Image3Db->GetDimension(2) == 8);
  }

  void SetDimensionT4D_Success()
  {
    CPPUNIT_ASSERT_MESSAGE("Testing if the T dimension of the 4D image is set correctly.", m_Image4Da->GetDimension(3) == 1);
    CPPUNIT_ASSERT_MESSAGE("Testing if the T dimension of the 4D image is set correctly.", m_Image4Db->GetDimension(3) == 2);
  }

  void SetDimensions3Dc_Success()
  {
    CPPUNIT_ASSERT_MESSAGE("Testing if image3Dc dimension x is set correctly.", m_Image3Dc->GetDimension(0) == 1);
    CPPUNIT_ASSERT_MESSAGE("Testing if image3Dc dimension y is set correctly.", m_Image3Dc->GetDimension(1) == 2);
    CPPUNIT_ASSERT_MESSAGE("Testing if image3Dc dimension z is set correctly.", m_Image3Dc->GetDimension(2) == 3);
  }

  void SetDataTypeFloat2D_Success()
  {
    CPPUNIT_ASSERT_MESSAGE("Testing if the data type for a float 3D image is set correctly.",
                      m_Image2Da->GetPixelType().GetComponentType() == itk::IOComponentEnum::FLOAT);
  }

  void SetDataTypeUChar2D_Success()
  {
    CPPUNIT_ASSERT_MESSAGE("Testing if the data type for a UChar 2D image is set correctly.",
                      m_Image2Db->GetPixelType().GetComponentType() == itk::IOComponentEnum::UCHAR);
  }

  void SetDataTypeInt3D_Success()
  {
    CPPUNIT_ASSERT_MESSAGE("Testing if the data type for a Int 3D image is set correctly.",
                      m_Image3Da->GetPixelType().GetComponentType() == itk::IOComponentEnum::INT);
  }

  void SetDataTypeDouble3D_Success()
  {
    CPPUNIT_ASSERT_MESSAGE("Testing if the data type for a Double 3D image is set correctly.",
                      m_Image3Db->GetPixelType().GetComponentType() == itk::IOComponentEnum::DOUBLE);
  }

  void SetDataTypeFloat4D_Success()
  {
    CPPUNIT_ASSERT_MESSAGE("Testing if the data type for a Float 4D image is set correctly.",
                      m_Image4Da->GetPixelType().GetComponentType() == itk::IOComponentEnum::FLOAT);
  }

  void SetDataTypeUChar4D_Success()
  {
    CPPUNIT_ASSERT_MESSAGE("Testing if the data type for a UChar 4D image is set correctly.",
                      m_Image4Db->GetPixelType().GetComponentType() == itk::IOComponentEnum::UCHAR);
  }

  void SetDataTypeUInt3D_Success()
  {
    CPPUNIT_ASSERT_MESSAGE("Testing if the data type for a UInt 3D image is set correctly.",
                      m_Image3Dc->GetPixelType().GetComponentType() == itk::IOComponentEnum::UINT);
  }

  void SetPixelTypeFloat2D_Success()
  {
    auto scalarType = itk::IOPixelEnum::SCALAR;
    CPPUNIT_ASSERT_MESSAGE("Testing if the pixel type for a Float 2D image is set correctly.",
                                        m_Image2Da->GetPixelType().GetPixelType() == scalarType);
  }

  void SetPixelTypeUChar2D_Success()
  {
    auto scalarType = itk::IOPixelEnum::SCALAR;
    CPPUNIT_ASSERT_MESSAGE("Testing if the pixel type for a UChar 2D image is set correctly.",
                                        m_Image2Db->GetPixelType().GetPixelType() == scalarType);
  }

  void SetPixelTypeInt3D_Success()
  {
    auto scalarType = itk::IOPixelEnum::SCALAR;
    CPPUNIT_ASSERT_MESSAGE("Testing if the pixel type for a Int 3D image is set correctly.",
                                      m_Image3Da->GetPixelType().GetPixelType() == scalarType);
  }

  void SetPixelTypeDouble3D_Success()
  {
    auto scalarType = itk::IOPixelEnum::SCALAR;
    CPPUNIT_ASSERT_MESSAGE("Testing if the pixel type for a Double 3D image is set correctly.",
                                         m_Image3Db->GetPixelType().GetPixelType() == scalarType);
  }

  void SetPixelTypeFloat4D_Success()
  {
    auto scalarType = itk::IOPixelEnum::SCALAR;
    CPPUNIT_ASSERT_MESSAGE("Testing if the pixel type for a Float 4D image is set correctly.",
                                        m_Image4Da->GetPixelType().GetPixelType() == scalarType);
  }

  void SetPixelTypeUChar4D_Success()
  {
    auto scalarType = itk::IOPixelEnum::SCALAR;
    CPPUNIT_ASSERT_MESSAGE("Testing if the pixel type for a UChar 4D image is set correctly.",
                                        m_Image4Db->GetPixelType().GetPixelType() == scalarType);
  }

  void SetPixelTypeUInt3D_Success()
  {
    auto scalarType = itk::IOPixelEnum::SCALAR;
    CPPUNIT_ASSERT_MESSAGE("Testing if the pixel type for a UInt 3D image is set correctly.",
                                       m_Image3Dc->GetPixelType().GetPixelType() == scalarType);
  }

  void MaxValueHolds_Success()
  {
    CPPUNIT_ASSERT_MESSAGE("Testing if max value holds", m_Image2Da->GetStatistics()->GetScalarValueMax() <= 577);
  }

  void MinValueHolds_Success()
  {
    CPPUNIT_ASSERT_MESSAGE("Testing if min value holds", m_Image2Da->GetStatistics()->GetScalarValueMin() >= 23);
  }

  void DefaultMaxValueHolds_Success()
  {
    CPPUNIT_ASSERT_MESSAGE("Testing if default max value holds", m_Image3Da->GetStatistics()->GetScalarValueMax() <= 1000);
  }

  void DefaultMinValueHolds_Success()
  {
    CPPUNIT_ASSERT_MESSAGE("Testing if default min value holds", m_Image3Da->GetStatistics()->GetScalarValueMin() >= 0);
  }

  void SetGradientImageValues_Success()
  {
    const unsigned int *image3DcBuffer = nullptr;
    try
    {
      mitk::ImageReadAccessor readAccess(m_Image3Dc);
      image3DcBuffer = static_cast<const unsigned int *>(readAccess.GetData());
    }
    catch (...)
    {
      MITK_ERROR << "Read access not granted on mitk::Image.";
    }
    for (unsigned int i = 0; i < 2 * 3; i++)
    {
      CPPUNIT_ASSERT_MESSAGE("Testing if gradient image values are set correctly", image3DcBuffer[i] == i);
    }
  }
};
MITK_TEST_SUITE_REGISTRATION(mitkImageGenerator)

