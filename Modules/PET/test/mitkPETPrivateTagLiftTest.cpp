/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

/*============================================================================

Producer-side regression for the PET vendor private-tag lift performed by
mitk::BaseDICOMReaderService when reading a PT series. The IBSI-SUV
benchmark (mitkPETIBSIBenchmarkTest) exercises this path only indirectly:
it asserts the downstream SUV, not the lifted properties, so dropping the
lift (or renaming a property) could leave the benchmark green while
silently skewing decay timing on real data.

This test loads the IBSI DROs that are built around vendor private
elements and asserts the lifted mitk.pet.* properties directly:

  DRO_3_3_0  Siemens private decay datetime  -> mitk.pet.SiemensDecayDateTime
  DRO_3_3_1  GE private scan datetime        -> mitk.pet.GEScanDateTime
  DRO_2_4    Philips SUV scale factor        -> mitk.pet.PhilipsSUVScale
  DRO_2_5    Philips activity scale factor   -> mitk.pet.PhilipsActivityScale

plus a gating check: the Philips DRO_2_4 carries no Siemens/GE private
block, so it must not gain those datetime properties.

Skips with exit 77 (ctest SKIP_RETURN_CODE) when the IBSI data dir baked
in at configure time (MITK_PET_IBSI_DATA_DIR) is empty, mirroring the
benchmark test.

============================================================================*/

#include <mitkIOUtil.h>
#include <mitkImage.h>
#include <mitkPreferenceListReaderOptionsFunctor.h>
#include <mitkTemporoSpatialStringProperty.h>

#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <cctype>
#include <cstdlib>
#include <string>

namespace
{
  std::string JoinPath(const std::string& a, const std::string& b)
  {
    if (a.empty()) return b;
    return (a.back() == '/' || a.back() == '\\') ? (a + b) : (a + '/' + b);
  }

  mitk::Image::Pointer LoadDROPet(const std::string& dataDir, const char* droId)
  {
    const std::string ptDir = JoinPath(JoinPath(JoinPath(dataDir, "DRO"), droId), "PT");
    mitk::PreferenceListReaderOptionsFunctor readerFunctor(
      { "MITK DICOM Reader v2 (autoselect)" }, { "" });
    return mitk::IOUtil::Load<mitk::Image>(ptDir, &readerFunctor);
  }

  const mitk::TemporoSpatialStringProperty*
  AsTemporoSpatial(const mitk::Image* image, const char* name)
  {
    auto prop = image->GetConstProperty(name);
    return dynamic_cast<const mitk::TemporoSpatialStringProperty*>(prop.GetPointer());
  }
}

class mitkPETPrivateTagLiftTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkPETPrivateTagLiftTestSuite);
  MITK_TEST(SiemensDecayDateTime_Lifted);
  MITK_TEST(GEScanDateTime_Lifted);
  MITK_TEST(PhilipsSUVScale_Lifted);
  MITK_TEST(PhilipsActivityScale_Lifted);
  MITK_TEST(NonVendorBlock_NotLifted);
  CPPUNIT_TEST_SUITE_END();

private:

  std::string m_DataDir;

  static std::string DataDirOrSkip()
  {
#ifndef MITK_PET_IBSI_DATA_DIR
#define MITK_PET_IBSI_DATA_DIR ""
#endif
    const std::string dataDir(MITK_PET_IBSI_DATA_DIR);
    if (dataDir.empty())
    {
      MITK_INFO << "MITK_PET_IBSI_DATA_DIR was empty at configure time; skipping "
                   "the PET private-tag lift test. Configure with "
                   "MITK_PET_DOWNLOAD_IBSI_DATA=ON or set MITK_PET_IBSI_DATA_DIR "
                   "and rebuild to opt in.";
      std::exit(77);
    }
    return dataDir;
  }

  // Assert that every populated (timestep, slice) slot of a lifted
  // datetime property carries a plausible DICOM DT (at least YYYYMMDD).
  static void AssertDateTimeProperty(const mitk::Image* image, const char* name)
  {
    const auto* prop = AsTemporoSpatial(image, name);
    CPPUNIT_ASSERT_MESSAGE(std::string("Expected lifted property '") + name + "'.",
                           nullptr != prop);

    const auto timeSteps = prop->GetAvailableTimeSteps();
    CPPUNIT_ASSERT_MESSAGE(std::string("Property '") + name + "' has no time steps.",
                           !timeSteps.empty());

    bool anySlot = false;
    for (const auto t : timeSteps)
    {
      for (const auto s : prop->GetAvailableSlices(t))
      {
        anySlot = true;
        const std::string v = prop->GetValue(t, s, false, false);
        CPPUNIT_ASSERT_MESSAGE(std::string("Empty datetime in '") + name + "'.",
                               v.size() >= 8);
        for (std::size_t i = 0; i < 8; ++i)
        {
          CPPUNIT_ASSERT_MESSAGE(std::string("Non-digit in DT prefix of '") + name + "'.",
                                 0 != std::isdigit(static_cast<unsigned char>(v[i])));
        }
      }
    }
    CPPUNIT_ASSERT_MESSAGE(std::string("Property '") + name + "' has no populated slots.",
                           anySlot);
  }

  // Assert that every populated slot of a lifted scale property parses to a
  // finite, positive double.
  static void AssertPositiveScaleProperty(const mitk::Image* image, const char* name)
  {
    const auto* prop = AsTemporoSpatial(image, name);
    CPPUNIT_ASSERT_MESSAGE(std::string("Expected lifted property '") + name + "'.",
                           nullptr != prop);

    bool anySlot = false;
    for (const auto t : prop->GetAvailableTimeSteps())
    {
      for (const auto s : prop->GetAvailableSlices(t))
      {
        anySlot = true;
        const std::string v = prop->GetValue(t, s, false, false);
        char* end = nullptr;
        const double d = std::strtod(v.c_str(), &end);
        CPPUNIT_ASSERT_MESSAGE(std::string("Unparseable scale in '") + name + "': " + v,
                               end != v.c_str());
        CPPUNIT_ASSERT_MESSAGE(std::string("Non-positive scale in '") + name + "'.",
                               d > 0.0);
      }
    }
    CPPUNIT_ASSERT_MESSAGE(std::string("Property '") + name + "' has no populated slots.",
                           anySlot);
  }

public:

  void setUp() override
  {
    m_DataDir = DataDirOrSkip();
  }

  void SiemensDecayDateTime_Lifted()
  {
    auto image = LoadDROPet(m_DataDir, "DRO_3_3_0");
    CPPUNIT_ASSERT(image.IsNotNull());
    AssertDateTimeProperty(image, "mitk.pet.SiemensDecayDateTime");
  }

  void GEScanDateTime_Lifted()
  {
    auto image = LoadDROPet(m_DataDir, "DRO_3_3_1");
    CPPUNIT_ASSERT(image.IsNotNull());
    AssertDateTimeProperty(image, "mitk.pet.GEScanDateTime");
  }

  void PhilipsSUVScale_Lifted()
  {
    auto image = LoadDROPet(m_DataDir, "DRO_2_4");
    CPPUNIT_ASSERT(image.IsNotNull());
    AssertPositiveScaleProperty(image, "mitk.pet.PhilipsSUVScale");
  }

  void PhilipsActivityScale_Lifted()
  {
    auto image = LoadDROPet(m_DataDir, "DRO_2_5");
    CPPUNIT_ASSERT(image.IsNotNull());
    AssertPositiveScaleProperty(image, "mitk.pet.PhilipsActivityScale");
  }

  void NonVendorBlock_NotLifted()
  {
    // DRO_2_4 is a Philips series with no Siemens / GE private block, so
    // the gate must not attach those vendor datetime properties.
    auto image = LoadDROPet(m_DataDir, "DRO_2_4");
    CPPUNIT_ASSERT(image.IsNotNull());
    CPPUNIT_ASSERT(image->GetConstProperty("mitk.pet.SiemensDecayDateTime").IsNull());
    CPPUNIT_ASSERT(image->GetConstProperty("mitk.pet.GEScanDateTime").IsNull());
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkPETPrivateTagLift)
