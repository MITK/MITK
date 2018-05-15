///*===================================================================

//The Medical Imaging Interaction Toolkit (MITK)

//Copyright (c) German Cancer Research Center,
//Division of Medical and Biological Informatics.
//All rights reserved.

//This software is distributed WITHOUT ANY WARRANTY; without
//even the implied warranty of MERCHANTABILITY or FITNESS FOR
//A PARTICULAR PURPOSE.

//See LICENSE.txt or http://www.mitk.org for details.

//===================================================================*/
#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <mitkPASpectralUnmixingFilterBase.h>

class mitkSpectralUnmixingTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkSpectralUnmixingTestSuite);
  MITK_TEST(testSUAlgorithm);
  CPPUNIT_TEST_SUITE_END();

private:
  mitk::pa::SpectralUnmixingFilterBase::Pointer m_SpectralUnmixingFilter;

public:

  void setUp() override
  {
  }

  void testSUAlgorithm()
  {
    // INSERT TEST HERE?
    MITK_INFO << "TEST TEST";



  }




  void tearDown() override
  {
    m_SpectralUnmixingFilter = nullptr;
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkSpectralUnmixing)


//++++++++++++++++++++++ erstmal (TM) verworfen:+++++++++++++++++++++++++++++++++++
/* relativ aufwenig ggf Bild laden
void creatInputImage()
{
unsigned int xDim = 1;
unsigned int yDim = 1;
unsigned int zDim = 3;

mitk::Image *inputImage;

mitk::PixelType pixelType = mitk::MakeScalarPixelType<float>();
const int NUMBER_OF_SPATIAL_DIMENSIONS = 3;
auto* dimensions = new unsigned int[NUMBER_OF_SPATIAL_DIMENSIONS];
dimensions[0] = xDim;
dimensions[1] = yDim;
dimensions[2] = zDim;

inputImage->Initialize(pixelType, NUMBER_OF_SPATIAL_DIMENSIONS, dimensions);

float value;
mitk::ImageWriteAccessor writeOutput(inputImage);


for (int x = 0; x < xDim; ++x)
{
for (int y = 0; y < yDim; ++y)
{
for (int z = 0; z < zDim; ++z)
{
float* writeBuffer = (float *)writeOutput.GetData();
writeBuffer[x*yDim + y] = value;
}
}
}


}/**/
