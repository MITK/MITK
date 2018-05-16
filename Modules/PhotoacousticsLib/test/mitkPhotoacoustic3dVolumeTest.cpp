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

#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include "mitkPAVolume.h"
#include <mitkImage.h>
#include <mitkImageReadAccessor.h>

class mitkPhotoacoustic3dVolumeTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkPhotoacoustic3dVolumeTestSuite);
  MITK_TEST(TestCorrectGetDataAndSetDataBehavior);
  MITK_TEST(TestCallingConstructorWithNullParameter);
  MITK_TEST(TestCallingConstructorWithCorrectParameters);
  MITK_TEST(TestModifyImage);
  MITK_TEST(TestModifyComplexImage);
  MITK_TEST(TestConvertToMitkImage);
  MITK_TEST(TestDeepCopy);
  MITK_TEST(TestCatchException);
  CPPUNIT_TEST_SUITE_END();

private:

  mitk::pa::Volume::Pointer m_Photoacoustic3dVolume;

public:

  void setUp() override
  {
  }

  void TestCallingConstructorWithNullParameter()
  {
    bool exceptionEncountered = false;
    try
    {
      m_Photoacoustic3dVolume = mitk::pa::Volume::New(nullptr, 3, 3, 3);
    }
    catch (...)
    {
      exceptionEncountered = true;
    }
    CPPUNIT_ASSERT(exceptionEncountered);
  }

  void TestCallingConstructorWithCorrectParameters()
  {
    auto* data = new double[1];
    data[0] = 3;
    m_Photoacoustic3dVolume = mitk::pa::Volume::New(data, 1, 1, 1);
    CPPUNIT_ASSERT(m_Photoacoustic3dVolume->GetData(0, 0, 0) == 3);
    CPPUNIT_ASSERT(m_Photoacoustic3dVolume->GetXDim() == 1);
    CPPUNIT_ASSERT(m_Photoacoustic3dVolume->GetYDim() == 1);
    CPPUNIT_ASSERT(m_Photoacoustic3dVolume->GetZDim() == 1);
  }

  void TestModifyImage()
  {
    auto* data = new double[1];
    data[0] = 3;
    m_Photoacoustic3dVolume = mitk::pa::Volume::New(data, 1, 1, 1);
    CPPUNIT_ASSERT_MESSAGE(std::to_string(m_Photoacoustic3dVolume->GetData(0, 0, 0)), m_Photoacoustic3dVolume->GetData(0, 0, 0) == 3);
    m_Photoacoustic3dVolume->SetData(17, 0, 0, 0);
    CPPUNIT_ASSERT(m_Photoacoustic3dVolume->GetData(0, 0, 0) == 17);
  }

  void TestModifyComplexImage()
  {
    unsigned int xDim = 4;
    unsigned int yDim = 7;
    unsigned int zDim = 12;
    unsigned int length = xDim * yDim * zDim;
    auto* data = new double[length];
    for (unsigned int i = 0; i < length; i++)
      data[i] = 5;

    m_Photoacoustic3dVolume = mitk::pa::Volume::New(data, xDim, yDim, zDim);

    for (unsigned int z = 0; z < zDim; z++)
      for (unsigned int y = 0; y < yDim; y++)
        for (unsigned int x = 0; x < xDim; x++)
        {
          CPPUNIT_ASSERT(m_Photoacoustic3dVolume->GetData(x, y, z) == 5);
          m_Photoacoustic3dVolume->SetData((x + y)*(z + 1), x, y, z);
          CPPUNIT_ASSERT(std::abs(m_Photoacoustic3dVolume->GetData(x, y, z) - (x + y)*(z + 1)) < mitk::eps);
        }
  }

  void TestCorrectGetDataAndSetDataBehavior()
  {
    unsigned int xDim = 40;
    unsigned int yDim = 7;
    unsigned int zDim = 12;
    unsigned int length = xDim * yDim * zDim;
    auto* data = new double[length];
    for (unsigned int i = 0; i < length; i++)
      data[i] = 0;

    m_Photoacoustic3dVolume = mitk::pa::Volume::New(data, xDim, yDim, zDim);

    for (unsigned int z = 0; z < zDim; z++)
      for (unsigned int y = 0; y < yDim; y++)
        for (unsigned int x = 0; x < xDim; x++)
        {
          int index = z*xDim*yDim + x*yDim + y;
          m_Photoacoustic3dVolume->SetData(index, x, y, z);
          CPPUNIT_ASSERT_MESSAGE(std::to_string(index), m_Photoacoustic3dVolume->GetData(x, y, z) == index);
        }
  }

  void TestConvertToMitkImage()
  {
    auto* data = new double[6];
    data[0] = 3;
    data[1] = 3;
    data[2] = 3;
    data[3] = 3;
    data[4] = 3;
    data[5] = 3;
    m_Photoacoustic3dVolume = mitk::pa::Volume::New(data, 1, 2, 3);
    CPPUNIT_ASSERT(m_Photoacoustic3dVolume->GetData(0, 0, 0) == 3);
    CPPUNIT_ASSERT(m_Photoacoustic3dVolume->GetData(0, 0, 1) == 3);
    CPPUNIT_ASSERT(m_Photoacoustic3dVolume->GetData(0, 0, 2) == 3);
    CPPUNIT_ASSERT(m_Photoacoustic3dVolume->GetData(0, 1, 0) == 3);
    CPPUNIT_ASSERT(m_Photoacoustic3dVolume->GetData(0, 1, 1) == 3);
    CPPUNIT_ASSERT(m_Photoacoustic3dVolume->GetData(0, 1, 2) == 3);

    m_Photoacoustic3dVolume->SetData(17, 0, 0, 0);
    m_Photoacoustic3dVolume->SetData(17, 0, 1, 0);
    m_Photoacoustic3dVolume->SetData(17, 0, 1, 2);
    CPPUNIT_ASSERT(m_Photoacoustic3dVolume->GetData(0, 0, 0) == 17);
    CPPUNIT_ASSERT(m_Photoacoustic3dVolume->GetData(0, 0, 1) == 3);
    CPPUNIT_ASSERT(m_Photoacoustic3dVolume->GetData(0, 0, 2) == 3);
    CPPUNIT_ASSERT(m_Photoacoustic3dVolume->GetData(0, 1, 0) == 17);
    CPPUNIT_ASSERT(m_Photoacoustic3dVolume->GetData(0, 1, 1) == 3);
    CPPUNIT_ASSERT(m_Photoacoustic3dVolume->GetData(0, 1, 2) == 17);

    mitk::Image::Pointer mitkImage = m_Photoacoustic3dVolume->AsMitkImage();
    CPPUNIT_ASSERT(mitkImage->GetDimensions()[0] == 2);
    CPPUNIT_ASSERT(mitkImage->GetDimensions()[1] == 1);
    CPPUNIT_ASSERT(mitkImage->GetDimensions()[2] == 3);

    mitk::ImageReadAccessor readAccess(mitkImage, mitkImage->GetVolumeData());
    auto* copyData = (double*)readAccess.GetData();
    CPPUNIT_ASSERT_MESSAGE(std::to_string(copyData[0]), copyData[0] == 17);
    CPPUNIT_ASSERT_MESSAGE(std::to_string(copyData[1]), copyData[1] == 17);
    CPPUNIT_ASSERT_MESSAGE(std::to_string(copyData[2]), copyData[2] == 3);
    CPPUNIT_ASSERT_MESSAGE(std::to_string(copyData[3]), copyData[3] == 3);
    CPPUNIT_ASSERT_MESSAGE(std::to_string(copyData[4]), copyData[4] == 3);
    CPPUNIT_ASSERT_MESSAGE(std::to_string(copyData[5]), copyData[5] == 17);
  }

  void TestDeepCopy()
  {
    auto* data = new double[1];
    data[0] = 3;
    m_Photoacoustic3dVolume = mitk::pa::Volume::New(data, 1, 1, 1);
    mitk::pa::Volume::Pointer copiedVolume = m_Photoacoustic3dVolume->DeepCopy();

    CPPUNIT_ASSERT(m_Photoacoustic3dVolume->GetXDim() == copiedVolume->GetXDim());
    CPPUNIT_ASSERT(m_Photoacoustic3dVolume->GetYDim() == copiedVolume->GetYDim());
    CPPUNIT_ASSERT(m_Photoacoustic3dVolume->GetZDim() == copiedVolume->GetZDim());
    CPPUNIT_ASSERT(m_Photoacoustic3dVolume->GetData(0, 0, 0) == 3);
    CPPUNIT_ASSERT(copiedVolume->GetData(0, 0, 0) == 3);
    m_Photoacoustic3dVolume->SetData(17, 0, 0, 0);
    CPPUNIT_ASSERT(m_Photoacoustic3dVolume->GetData(0, 0, 0) == 17);
    CPPUNIT_ASSERT(copiedVolume->GetData(0, 0, 0) == 3);
  }

  void AssertIndexException(unsigned int x, unsigned int y, unsigned int z)
  {
    bool exceptionCaught = false;
    try
    {
      double thisIsIrrelevant = m_Photoacoustic3dVolume->GetData(x, y, z);
      thisIsIrrelevant += 1;
    }
    catch (...)
    {
      exceptionCaught = true;
      if (exceptionCaught)
        exceptionCaught = true;
    }
#ifdef _DEBUG

    CPPUNIT_ASSERT(exceptionCaught);

#endif
    }

  void TestCatchException()
  {
    auto* data = new double[1];
    data[0] = 3;
    m_Photoacoustic3dVolume = mitk::pa::Volume::New(data, 1, 1, 1);

    AssertIndexException(1, 0, 0);
    AssertIndexException(0, 1, 0);
    AssertIndexException(0, 0, 1);
    AssertIndexException(18, 1, 222);
  }

  void tearDown() override
  {
    m_Photoacoustic3dVolume = nullptr;
  }
  };

MITK_TEST_SUITE_REGISTRATION(mitkPhotoacoustic3dVolume)
