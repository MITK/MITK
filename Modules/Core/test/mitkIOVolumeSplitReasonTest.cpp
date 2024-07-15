/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkTestingMacros.h"
#include <mitkTestFixture.h>
#include <mitkTestingConfig.h>

#include <mitkIOVolumeSplitReason.h>
#include <mitkVersion.h>


class mitkIOVolumeSplitReasonTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkIOVolumeSplitReasonTestSuite);
  MITK_TEST(TestAddNRemoveMethods);
  MITK_TEST(TestSerializeReasonType);
  MITK_TEST(TestSerializeReason);
  MITK_TEST(TestExtendReason);
  CPPUNIT_TEST_SUITE_END();

private:
  std::string m_ReasonStr;
  mitk::IOVolumeSplitReason::Pointer m_Reason1;
  mitk::IOVolumeSplitReason::Pointer m_Reason2;
  mitk::IOVolumeSplitReason::Pointer m_EmptyReason;

public:
  void setUp() override
  {
    m_Reason1 = mitk::IOVolumeSplitReason::New();
    m_Reason2 = mitk::IOVolumeSplitReason::New();
    m_EmptyReason = mitk::IOVolumeSplitReason::New();

    m_Reason1->AddReason(mitk::IOVolumeSplitReason::ReasonType::OverlappingSlices);
    m_Reason1->AddReason(mitk::IOVolumeSplitReason::ReasonType::MissingSlices,"1");
    m_Reason2->AddReason(mitk::IOVolumeSplitReason::ReasonType::ImagePostionMissing);
    m_Reason2->AddReason(mitk::IOVolumeSplitReason::ReasonType::MissingSlices, "2");

    m_ReasonStr = "[[\"value_sort_distance\",\"detail\"],[\"overlapping_slices\"],[\"missing_slices\",\"3\"]]";
  }

  void TestAddNRemoveMethods()
  {
    CPPUNIT_ASSERT(!m_EmptyReason->ReasonExists());
    CPPUNIT_ASSERT(m_Reason1->ReasonExists());
    CPPUNIT_ASSERT(m_Reason2->ReasonExists());

    CPPUNIT_ASSERT(!m_Reason1->ReasonExists(mitk::IOVolumeSplitReason::ReasonType::GantryTiltDifference));
    CPPUNIT_ASSERT(!m_Reason1->ReasonExists(mitk::IOVolumeSplitReason::ReasonType::ImagePostionMissing));
    CPPUNIT_ASSERT(m_Reason1->ReasonExists(mitk::IOVolumeSplitReason::ReasonType::MissingSlices));
    CPPUNIT_ASSERT(m_Reason1->ReasonExists(mitk::IOVolumeSplitReason::ReasonType::OverlappingSlices));
    CPPUNIT_ASSERT(!m_Reason1->ReasonExists(mitk::IOVolumeSplitReason::ReasonType::SliceDistanceInconsistency));
    CPPUNIT_ASSERT(!m_Reason1->ReasonExists(mitk::IOVolumeSplitReason::ReasonType::Unkown));
    CPPUNIT_ASSERT(!m_Reason1->ReasonExists(mitk::IOVolumeSplitReason::ReasonType::ValueSortDistance));
    CPPUNIT_ASSERT(!m_Reason1->ReasonExists(mitk::IOVolumeSplitReason::ReasonType::ValueSplitDifference));

    CPPUNIT_ASSERT(!m_Reason2->ReasonExists(mitk::IOVolumeSplitReason::ReasonType::GantryTiltDifference));
    CPPUNIT_ASSERT(m_Reason2->ReasonExists(mitk::IOVolumeSplitReason::ReasonType::ImagePostionMissing));
    CPPUNIT_ASSERT(m_Reason2->ReasonExists(mitk::IOVolumeSplitReason::ReasonType::MissingSlices));
    CPPUNIT_ASSERT(!m_Reason2->ReasonExists(mitk::IOVolumeSplitReason::ReasonType::OverlappingSlices));
    CPPUNIT_ASSERT(!m_Reason2->ReasonExists(mitk::IOVolumeSplitReason::ReasonType::SliceDistanceInconsistency));
    CPPUNIT_ASSERT(!m_Reason2->ReasonExists(mitk::IOVolumeSplitReason::ReasonType::Unkown));
    CPPUNIT_ASSERT(!m_Reason2->ReasonExists(mitk::IOVolumeSplitReason::ReasonType::ValueSortDistance));
    CPPUNIT_ASSERT(!m_Reason2->ReasonExists(mitk::IOVolumeSplitReason::ReasonType::ValueSplitDifference));

    m_Reason1->RemoveReason(mitk::IOVolumeSplitReason::ReasonType::Unkown);

    CPPUNIT_ASSERT(!m_Reason1->ReasonExists(mitk::IOVolumeSplitReason::ReasonType::GantryTiltDifference));
    CPPUNIT_ASSERT(!m_Reason1->ReasonExists(mitk::IOVolumeSplitReason::ReasonType::ImagePostionMissing));
    CPPUNIT_ASSERT(m_Reason1->ReasonExists(mitk::IOVolumeSplitReason::ReasonType::MissingSlices));
    CPPUNIT_ASSERT(m_Reason1->ReasonExists(mitk::IOVolumeSplitReason::ReasonType::OverlappingSlices));
    CPPUNIT_ASSERT(!m_Reason1->ReasonExists(mitk::IOVolumeSplitReason::ReasonType::SliceDistanceInconsistency));
    CPPUNIT_ASSERT(!m_Reason1->ReasonExists(mitk::IOVolumeSplitReason::ReasonType::Unkown));
    CPPUNIT_ASSERT(!m_Reason1->ReasonExists(mitk::IOVolumeSplitReason::ReasonType::ValueSortDistance));
    CPPUNIT_ASSERT(!m_Reason1->ReasonExists(mitk::IOVolumeSplitReason::ReasonType::ValueSplitDifference));

    m_Reason1->RemoveReason(mitk::IOVolumeSplitReason::ReasonType::MissingSlices);

    CPPUNIT_ASSERT(!m_Reason1->ReasonExists(mitk::IOVolumeSplitReason::ReasonType::GantryTiltDifference));
    CPPUNIT_ASSERT(!m_Reason1->ReasonExists(mitk::IOVolumeSplitReason::ReasonType::ImagePostionMissing));
    CPPUNIT_ASSERT(!m_Reason1->ReasonExists(mitk::IOVolumeSplitReason::ReasonType::MissingSlices));
    CPPUNIT_ASSERT(m_Reason1->ReasonExists(mitk::IOVolumeSplitReason::ReasonType::OverlappingSlices));
    CPPUNIT_ASSERT(!m_Reason1->ReasonExists(mitk::IOVolumeSplitReason::ReasonType::SliceDistanceInconsistency));
    CPPUNIT_ASSERT(!m_Reason1->ReasonExists(mitk::IOVolumeSplitReason::ReasonType::Unkown));
    CPPUNIT_ASSERT(!m_Reason1->ReasonExists(mitk::IOVolumeSplitReason::ReasonType::ValueSortDistance));
    CPPUNIT_ASSERT(!m_Reason1->ReasonExists(mitk::IOVolumeSplitReason::ReasonType::ValueSplitDifference));

    m_Reason1->AddReason(mitk::IOVolumeSplitReason::ReasonType::ValueSortDistance);
    m_Reason1->AddReason(mitk::IOVolumeSplitReason::ReasonType::ValueSplitDifference);

    CPPUNIT_ASSERT(!m_Reason1->ReasonExists(mitk::IOVolumeSplitReason::ReasonType::GantryTiltDifference));
    CPPUNIT_ASSERT(!m_Reason1->ReasonExists(mitk::IOVolumeSplitReason::ReasonType::ImagePostionMissing));
    CPPUNIT_ASSERT(!m_Reason1->ReasonExists(mitk::IOVolumeSplitReason::ReasonType::MissingSlices));
    CPPUNIT_ASSERT(m_Reason1->ReasonExists(mitk::IOVolumeSplitReason::ReasonType::OverlappingSlices));
    CPPUNIT_ASSERT(!m_Reason1->ReasonExists(mitk::IOVolumeSplitReason::ReasonType::SliceDistanceInconsistency));
    CPPUNIT_ASSERT(!m_Reason1->ReasonExists(mitk::IOVolumeSplitReason::ReasonType::Unkown));
    CPPUNIT_ASSERT(m_Reason1->ReasonExists(mitk::IOVolumeSplitReason::ReasonType::ValueSortDistance));
    CPPUNIT_ASSERT(m_Reason1->ReasonExists(mitk::IOVolumeSplitReason::ReasonType::ValueSplitDifference));
  }

  static bool CheckType(mitk::IOVolumeSplitReason::ReasonType reasontype, const std::string& reasonStr)
  {
    if (mitk::IOVolumeSplitReason::TypeToString(reasontype) != reasonStr)
      return false;
    if (mitk::IOVolumeSplitReason::StringToType(reasonStr) != reasontype)
      return false;

    return true;
  }

  void TestSerializeReasonType()
  {
    CPPUNIT_ASSERT(CheckType(mitk::IOVolumeSplitReason::ReasonType::GantryTiltDifference,"gantry_tilt_difference"));
    CPPUNIT_ASSERT(CheckType(mitk::IOVolumeSplitReason::ReasonType::ImagePostionMissing, "image_position_missing"));
    CPPUNIT_ASSERT(CheckType(mitk::IOVolumeSplitReason::ReasonType::MissingSlices, "missing_slices"));
    CPPUNIT_ASSERT(CheckType(mitk::IOVolumeSplitReason::ReasonType::OverlappingSlices, "overlapping_slices"));
    CPPUNIT_ASSERT(CheckType(mitk::IOVolumeSplitReason::ReasonType::SliceDistanceInconsistency, "slice_distance_inconsistency"));
    CPPUNIT_ASSERT(CheckType(mitk::IOVolumeSplitReason::ReasonType::Unkown, "unknown"));
    CPPUNIT_ASSERT(CheckType(mitk::IOVolumeSplitReason::ReasonType::ValueSortDistance, "value_sort_distance"));
    CPPUNIT_ASSERT(CheckType(mitk::IOVolumeSplitReason::ReasonType::ValueSplitDifference, "value_split_difference"));
  }

  void TestSerializeReason()
  {
    auto serializedReason1 = mitk::IOVolumeSplitReason::SerializeToJSON(m_Reason1);
    auto serializedReason2 = mitk::IOVolumeSplitReason::SerializeToJSON(m_Reason2);
    auto serializedReasonEmpty = mitk::IOVolumeSplitReason::SerializeToJSON(m_EmptyReason);

    CPPUNIT_ASSERT("[[\"overlapping_slices\"],[\"missing_slices\",\"1\"]]" == serializedReason1);
    CPPUNIT_ASSERT("[[\"image_position_missing\"],[\"missing_slices\",\"2\"]]" == serializedReason2);
    CPPUNIT_ASSERT("[]" == serializedReasonEmpty);

    auto newReason = mitk::IOVolumeSplitReason::DeserializeFromJSON(m_ReasonStr);
    CPPUNIT_ASSERT(!newReason->ReasonExists(mitk::IOVolumeSplitReason::ReasonType::GantryTiltDifference));
    CPPUNIT_ASSERT(!newReason->ReasonExists(mitk::IOVolumeSplitReason::ReasonType::ImagePostionMissing));
    CPPUNIT_ASSERT(newReason->ReasonExists(mitk::IOVolumeSplitReason::ReasonType::MissingSlices));
    CPPUNIT_ASSERT(newReason->ReasonExists(mitk::IOVolumeSplitReason::ReasonType::OverlappingSlices));
    CPPUNIT_ASSERT(!newReason->ReasonExists(mitk::IOVolumeSplitReason::ReasonType::SliceDistanceInconsistency));
    CPPUNIT_ASSERT(!newReason->ReasonExists(mitk::IOVolumeSplitReason::ReasonType::Unkown));
    CPPUNIT_ASSERT(newReason->ReasonExists(mitk::IOVolumeSplitReason::ReasonType::ValueSortDistance));
    CPPUNIT_ASSERT(!newReason->ReasonExists(mitk::IOVolumeSplitReason::ReasonType::ValueSplitDifference));

    CPPUNIT_ASSERT(newReason->GetReasonDetails(mitk::IOVolumeSplitReason::ReasonType::ValueSortDistance) == "detail");
    CPPUNIT_ASSERT(newReason->GetReasonDetails(mitk::IOVolumeSplitReason::ReasonType::MissingSlices) == "3");
    CPPUNIT_ASSERT(newReason->GetReasonDetails(mitk::IOVolumeSplitReason::ReasonType::OverlappingSlices) == "");
  }

  void TestExtendReason()
  {
    auto extendedReason = m_Reason1->ExtendReason(m_EmptyReason);
    CPPUNIT_ASSERT("[[\"overlapping_slices\"],[\"missing_slices\",\"1\"]]" == mitk::IOVolumeSplitReason::SerializeToJSON(extendedReason));

    extendedReason = m_EmptyReason->ExtendReason(m_Reason1);
    CPPUNIT_ASSERT("[[\"overlapping_slices\"],[\"missing_slices\",\"1\"]]" == mitk::IOVolumeSplitReason::SerializeToJSON(extendedReason));

    extendedReason = m_Reason2->ExtendReason(m_Reason1);
    CPPUNIT_ASSERT("[[\"image_position_missing\"],[\"overlapping_slices\"],[\"missing_slices\",\"2\"]]" == mitk::IOVolumeSplitReason::SerializeToJSON(extendedReason));
  }

};

MITK_TEST_SUITE_REGISTRATION(mitkIOVolumeSplitReason)
