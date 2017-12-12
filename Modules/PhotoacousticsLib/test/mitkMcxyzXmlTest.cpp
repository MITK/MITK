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

#include <mitkPAProbe.h>

// us
#include <usModule.h>
#include <usModuleResource.h>
#include <usGetModuleContext.h>
#include <usModuleContext.h>
#include <usModuleResourceStream.h>
#include <mitkLocaleSwitch.h>
#include <random>
#include <chrono>

class mitkMcxyzXmlTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkMcxyzXmlTestSuite);
  MITK_TEST(TestCreatePointSourceProbe);
  MITK_TEST(TestCreateCircleSourceProbe);
  MITK_TEST(TestCreateRectangleSourceProbe);
  MITK_TEST(TestCreateTwoPointSourcesProbe);
  MITK_TEST(TestCreateAllSourcesProbe);
  MITK_TEST(TestValuesAreInBoundsUniformRectangle);
  MITK_TEST(TestValuesAreInBoundsGaussianRectangle);
  CPPUNIT_TEST_SUITE_END();

private:

  mitk::pa::LightSource::Pointer m_LightSource;
  mitk::pa::Probe::Pointer m_Probe;
  std::string m_XmlProbePointSource;
  std::string m_XmlProbeCircleSource;
  std::string m_XmlProbeRectangleSource;
  std::string m_XmlProbeTwoPointSources;
  std::string m_XmlProbeAllSources;

public:

  void setUp() override
  {
    m_LightSource = mitk::pa::LightSource::New();
    LoadXmlFile("pointsource.xml", &m_XmlProbePointSource);
    LoadXmlFile("circlesource.xml", &m_XmlProbeCircleSource);
    LoadXmlFile("rectanglesource.xml", &m_XmlProbeRectangleSource);
    LoadXmlFile("twopointsources.xml", &m_XmlProbeTwoPointSources);
    LoadXmlFile("allsources.xml", &m_XmlProbeAllSources);
  }

  void LoadXmlFile(std::string filename, std::string* lines)
  {
    us::ModuleResource pointSourceXml = us::GetModuleContext()->GetModule()->GetResource(filename);
    std::string line;
    if (pointSourceXml.IsValid() && pointSourceXml.IsFile())
    {
      us::ModuleResourceStream stream(pointSourceXml);
      stream.std::istream::imbue(std::locale("C"));
      while (std::getline(stream, line))
      {
        *lines = *lines + line + " ";
      }
    }
    else
    {
      MITK_ERROR << "Xml file was not valid";
    }
  }

  void TestCreatePointSourceProbe()
  {
    m_Probe = mitk::pa::Probe::New(m_XmlProbePointSource.c_str(), true);
    CPPUNIT_ASSERT(true == m_Probe->IsValid());
  }

  void TestCreateCircleSourceProbe()
  {
    m_Probe = mitk::pa::Probe::New(m_XmlProbeCircleSource.c_str(), true);
    CPPUNIT_ASSERT(true == m_Probe->IsValid());
  }

  void TestCreateRectangleSourceProbe()
  {
    m_Probe = mitk::pa::Probe::New(m_XmlProbeRectangleSource.c_str(), true);
    CPPUNIT_ASSERT(true == m_Probe->IsValid());
  }

  void TestCreateTwoPointSourcesProbe()
  {
    m_Probe = mitk::pa::Probe::New(m_XmlProbeTwoPointSources.c_str(), true);
    CPPUNIT_ASSERT(true == m_Probe->IsValid());
  }

  void TestCreateAllSourcesProbe()
  {
    m_Probe = mitk::pa::Probe::New(m_XmlProbeAllSources.c_str(), true);
    CPPUNIT_ASSERT(true == m_Probe->IsValid());
  }

  void TestValuesAreInBoundsUniformRectangle()
  {
    int MAXIMUM = 2;
    int MINIMUM = -2;

    int ANGLE_MAXIMUM = 1;
    int ANGLE_MINIMUM = -1;

    m_LightSource->SetAngleXMode(mitk::pa::LightSource::DistributionMode::UNIFORM);
    m_LightSource->SetAngleYMode(mitk::pa::LightSource::DistributionMode::UNIFORM);

    m_LightSource->SetAngleXMaximum(ANGLE_MAXIMUM);
    m_LightSource->SetAngleXMinimum(ANGLE_MINIMUM);

    m_LightSource->SetAngleYMaximum(ANGLE_MAXIMUM);
    m_LightSource->SetAngleYMinimum(ANGLE_MINIMUM);

    m_LightSource->SetSpawnLocationX(MINIMUM);
    m_LightSource->SetSpawnLocationXLength(2 * MAXIMUM);

    m_LightSource->SetSpawnLocationY(MINIMUM);
    m_LightSource->SetSpawnLocationYLength(2 * MAXIMUM);

    m_LightSource->SetSpawnLocationZ(MINIMUM);
    m_LightSource->SetSpawnLocationZLength(2 * MAXIMUM);

    m_LightSource->SetSpawnLocationRadius(MAXIMUM);

    m_LightSource->SetVerbose(false);

    m_LightSource->SetSpawnType(mitk::pa::LightSource::SpawnType::RECTANGLE);

    std::mt19937 rng;
    rng.seed(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::
      now().time_since_epoch()).count());
    std::uniform_real_distribution<> realDist(0, 1);

    for (int i = 0, length = 10000; i < length; i++)
    {
      auto result =
        m_LightSource->GetNextPhoton(realDist(rng), realDist(rng), realDist(rng), realDist(rng),
          realDist(rng), realDist(rng), realDist(rng));

      CPPUNIT_ASSERT(result.xAngle >= ANGLE_MINIMUM);
      CPPUNIT_ASSERT(result.xAngle <= ANGLE_MAXIMUM);
      CPPUNIT_ASSERT(result.yAngle >= ANGLE_MINIMUM);
      CPPUNIT_ASSERT(result.yAngle <= ANGLE_MAXIMUM);
      CPPUNIT_ASSERT(result.zAngle >= 0);
      CPPUNIT_ASSERT(result.zAngle <= ANGLE_MAXIMUM);
      CPPUNIT_ASSERT(result.xPosition >= MINIMUM);
      CPPUNIT_ASSERT(result.xPosition <= MAXIMUM);
      CPPUNIT_ASSERT(result.yPosition >= MINIMUM);
      CPPUNIT_ASSERT(result.yPosition <= MAXIMUM);
      CPPUNIT_ASSERT(result.zPosition >= MINIMUM);
      CPPUNIT_ASSERT(result.zPosition <= MAXIMUM);
    }
  }

  void TestValuesAreInBoundsGaussianRectangle()
  {
    int MAXIMUM = 2;
    int MINIMUM = -2;

    int ANGLE_MAXIMUM = 1;
    int ANGLE_MINIMUM = -1;

    m_LightSource->SetAngleXMode(mitk::pa::LightSource::DistributionMode::GAUSSIAN);
    m_LightSource->SetAngleYMode(mitk::pa::LightSource::DistributionMode::GAUSSIAN);

    m_LightSource->SetAngleXMaximum(ANGLE_MAXIMUM);
    m_LightSource->SetAngleXMinimum(ANGLE_MINIMUM);

    m_LightSource->SetAngleYMaximum(ANGLE_MAXIMUM);
    m_LightSource->SetAngleYMinimum(ANGLE_MINIMUM);

    m_LightSource->SetSpawnLocationX(MINIMUM);
    m_LightSource->SetSpawnLocationXLength(2 * MAXIMUM);

    m_LightSource->SetSpawnLocationY(MINIMUM);
    m_LightSource->SetSpawnLocationYLength(2 * MAXIMUM);

    m_LightSource->SetSpawnLocationZ(MINIMUM);
    m_LightSource->SetSpawnLocationZLength(2 * MAXIMUM);

    m_LightSource->SetSpawnLocationRadius(MAXIMUM);

    m_LightSource->SetVerbose(false);

    m_LightSource->SetSpawnType(mitk::pa::LightSource::SpawnType::RECTANGLE);

    std::mt19937 rng;
    rng.seed(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::
      now().time_since_epoch()).count());
    std::uniform_real_distribution<> realDist(0, 1);

    for (int i = 0, length = 10000; i < length; i++)
    {
      auto result =
        m_LightSource->GetNextPhoton(realDist(rng), realDist(rng), realDist(rng), realDist(rng),
          realDist(rng), realDist(rng), realDist(rng));

      CPPUNIT_ASSERT(result.xAngle >= ANGLE_MINIMUM);
      CPPUNIT_ASSERT(result.xAngle <= ANGLE_MAXIMUM);
      CPPUNIT_ASSERT(result.yAngle >= ANGLE_MINIMUM);
      CPPUNIT_ASSERT(result.yAngle <= ANGLE_MAXIMUM);
      CPPUNIT_ASSERT(result.zAngle >= 0);
      CPPUNIT_ASSERT(result.zAngle <= ANGLE_MAXIMUM);
      CPPUNIT_ASSERT(result.xPosition >= MINIMUM);
      CPPUNIT_ASSERT(result.xPosition <= MAXIMUM);
      CPPUNIT_ASSERT(result.yPosition >= MINIMUM);
      CPPUNIT_ASSERT(result.yPosition <= MAXIMUM);
      CPPUNIT_ASSERT(result.zPosition >= MINIMUM);
      CPPUNIT_ASSERT(result.zPosition <= MAXIMUM);
    }
  }

  void tearDown() override
  {
    m_XmlProbePointSource = "";
    m_XmlProbeCircleSource = "";
    m_XmlProbeRectangleSource = "";
    m_XmlProbeTwoPointSources = "";
    m_XmlProbeAllSources = "";
    m_Probe = nullptr;
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkMcxyzXml)
