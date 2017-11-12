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

#include <mitkPAPropertyCalculator.h>

class mitkPropertyCalculatorTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkPropertyCalculatorTestSuite);
  MITK_TEST(testOutputIsNotEmpty);
  CPPUNIT_TEST_SUITE_END();

private:
  mitk::pa::PropertyCalculator::Pointer m_PropertyCalculator;

public:

  void setUp() override
  {
    m_PropertyCalculator = mitk::pa::PropertyCalculator::New();
  }

  void assertProperties(mitk::pa::PropertyCalculator::Properties properties,
    std::string type, int wavelength, int oxygenation)
  {
    CPPUNIT_ASSERT_MESSAGE("anisotropy " + std::to_string(properties.g) + " not bigger than 0 at type "
      + type + " wv: " + std::to_string(wavelength) + " ox: " + std::to_string(oxygenation), properties.g > 0);
    CPPUNIT_ASSERT_MESSAGE("absorption " + std::to_string(properties.mua) + " not bigger than 0 at type "
      + type + " wv: " + std::to_string(wavelength) + " ox: " + std::to_string(oxygenation), properties.mua > 0);
    CPPUNIT_ASSERT_MESSAGE("scattering " + std::to_string(properties.mus) + " not bigger than 0 at type "
      + type + " wv: " + std::to_string(wavelength) + " ox: " + std::to_string(oxygenation), properties.mus > 0);
  }

  void testOutputIsNotEmpty()
  {
    for (double oxygenation = 0; oxygenation <= 1; oxygenation += 0.05)
    {
      for (int wavelength = 700; wavelength <= 900; wavelength += 5)
      {
        auto properties = m_PropertyCalculator->CalculatePropertyForSpecificWavelength(
          mitk::pa::PropertyCalculator::TissueType::AIR, wavelength, oxygenation);
        assertProperties(properties, "AIR", wavelength, oxygenation);
        properties = m_PropertyCalculator->CalculatePropertyForSpecificWavelength(
          mitk::pa::PropertyCalculator::TissueType::BLOOD, wavelength, oxygenation);
        assertProperties(properties, "BLOOD", wavelength, oxygenation);
        properties = m_PropertyCalculator->CalculatePropertyForSpecificWavelength(
          mitk::pa::PropertyCalculator::TissueType::EPIDERMIS, wavelength, oxygenation);
        assertProperties(properties, "EPIDERMIS", wavelength, oxygenation);
        properties = m_PropertyCalculator->CalculatePropertyForSpecificWavelength(
          mitk::pa::PropertyCalculator::TissueType::FAT, wavelength, oxygenation);
        assertProperties(properties, "FAT", wavelength, oxygenation);
        properties = m_PropertyCalculator->CalculatePropertyForSpecificWavelength(
          mitk::pa::PropertyCalculator::TissueType::STANDARD_TISSUE, wavelength, oxygenation);
        assertProperties(properties, "STANDARD_TISSUE", wavelength, oxygenation);
      }
    }
  }

  void tearDown() override
  {
    m_PropertyCalculator = nullptr;
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkPropertyCalculator)
