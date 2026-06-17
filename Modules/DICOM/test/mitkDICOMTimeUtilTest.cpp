/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkDICOMTimeUtil.h>

#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <dcmtk/dcmdata/dcvrdt.h>

#include <string>

class mitkDICOMTimeUtilTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkDICOMTimeUtilTestSuite);
  MITK_TEST(Duration_NoTimeZone_Baseline);
  MITK_TEST(Duration_EqualTimeZone_Cancels);
  MITK_TEST(Duration_MixedTimeZone_NormalizedToUTC);
  MITK_TEST(Duration_FractionalTimeZone_NormalizedToUTC);
  CPPUNIT_TEST_SUITE_END();

private:

  // Parse a DICOM DT string ("YYYYMMDDHHMMSS[.FFFFFF][+-ZZXX]") into an
  // OFDateTime, failing the test if the string is not valid DT.
  static OFDateTime Parse(const std::string& dt)
  {
    OFDateTime out;
    const bool ok = DcmDateTime::getOFDateTimeFromString(OFString(dt.c_str()), out).good();
    CPPUNIT_ASSERT_MESSAGE("Failed to parse DICOM DT '" + dt + "'", ok);
    return out;
  }

public:

  void Duration_NoTimeZone_Baseline()
  {
    // Two offset-less stamps one hour apart difference to exactly 1 h. This
    // is the common single-timezone path and must stay bit-stable.
    const double ms = mitk::ComputeMiliSecDuration(
      Parse("20240101120000"), Parse("20240101130000"));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(3600000.0, ms, 1e-6);
  }

  void Duration_EqualTimeZone_Cancels()
  {
    // Equal explicit offsets cancel in the difference.
    const double ms = mitk::ComputeMiliSecDuration(
      Parse("20240101120000+0100"), Parse("20240101130000+0100"));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(3600000.0, ms, 1e-6);
  }

  void Duration_MixedTimeZone_NormalizedToUTC()
  {
    // Injection 12:00 at +01:00 is 11:00 UTC; the offset-less reference
    // 12:00 is treated as UTC. (reference - injection) is therefore +1 h.
    // Before the UTC-offset normalization this collapsed to 0.
    const double ms = mitk::ComputeMiliSecDuration(
      Parse("20240101120000+0100"), Parse("20240101120000"));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(3600000.0, ms, 1e-6);
  }

  void Duration_FractionalTimeZone_NormalizedToUTC()
  {
    // Injection 12:00 at +05:30 is 06:30 UTC; the offset-less reference
    // 06:30 is treated as UTC, so the decay duration is exactly 0.
    const double ms = mitk::ComputeMiliSecDuration(
      Parse("20240101120000+0530"), Parse("20240101063000"));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, ms, 1e-6);
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkDICOMTimeUtil)
