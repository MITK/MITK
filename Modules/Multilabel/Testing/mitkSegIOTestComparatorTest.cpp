/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkSegIOTestComparator.h>
#include <mitkSegIOTestFixtures.h>

#include <mitkImageWriteAccessor.h>
#include <mitkStringProperty.h>
#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <algorithm>

class mitkSegIOTestComparatorTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkSegIOTestComparatorTestSuite);
  MITK_TEST(IdenticalSegmentationsHaveNoDifferences);
  MITK_TEST(LabelPropertyValueIsDetected);
  MITK_TEST(LostLabelPropertyIsDetected);
  MITK_TEST(SparseAnatomicRegionIsDetectedWithoutThrowing);
  MITK_TEST(MissingAndUnexpectedLabelsAreDetected);
  MITK_TEST(ColorToleranceIsApplied);
  MITK_TEST(GroupNameIsDetected);
  MITK_TEST(VoxelIsDetected);
  MITK_TEST(VoxelIsDetectedInLaterTimeStep);
  MITK_TEST(StrayValueOutsideLabelsIsDetected);
  MITK_TEST(EmptyGroupImageIsDetected);
  MITK_TEST(SegmentationPropertyIsDetected);
  MITK_TEST(PropertyExclusionsAreApplied);
  MITK_TEST(UIDIsDetectedUnlessNotCompared);
  MITK_TEST(UnlabeledLockIsDetected);
  MITK_TEST(GeometryToleranceIsApplied);
  MITK_TEST(TimeBoundsAreDetected);
  MITK_TEST(RelationIsDetected);
  MITK_TEST(DocumentedLossIsApplied);
  MITK_TEST(StaleLossFails);
  CPPUNIT_TEST_SUITE_END();

  mitk::MultiLabelSegmentation::Pointer m_Original;
  mitk::MultiLabelSegmentation::Pointer m_Copy;

  static bool Mentions(const std::vector<std::string>& differences, const std::string& text)
  {
    return std::any_of(differences.begin(), differences.end(),
      [&text](const std::string& difference) { return difference.find(text) != std::string::npos; });
  }

  void AssertDetected(const std::string& text, const mitk::test::SegComparisonOptions& options = {})
  {
    const auto differences = mitk::test::CompareSegmentations(*m_Original, *m_Copy, options);
    CPPUNIT_ASSERT_MESSAGE("Expected a difference mentioning \"" + text + "\"; got:" +
                             mitk::test::JoinDifferences(differences),
                           Mentions(differences, text));
  }

  void AssertNoDifference(const mitk::test::SegComparisonOptions& options = {})
  {
    const auto differences = mitk::test::CompareSegmentations(*m_Original, *m_Copy, options);
    CPPUNIT_ASSERT_MESSAGE("Expected no differences; got:" + mitk::test::JoinDifferences(differences),
                           differences.empty());
  }

  void SetVoxel(mitk::MultiLabelSegmentation::GroupIndexType group, unsigned int timeStep, unsigned int x,
                mitk::Label::PixelType value)
  {
    namespace d = mitk::test::detail;
    auto* image = m_Copy->GetGroupImage(group);
    mitk::ImageWriteAccessor accessor(image);
    auto* voxels = static_cast<mitk::Label::PixelType*>(accessor.GetData());
    voxels[(timeStep * d::RichSizeZ * d::RichSizeY) * d::RichSizeX + x] = value;
  }

public:
  void setUp() override
  {
    m_Original = mitk::test::BuildRichSegmentation({ false, false, false, false });
    m_Copy = mitk::test::CloneKeepingUID(*m_Original);
  }

  void tearDown() override
  {
    m_Original = nullptr;
    m_Copy = nullptr;
  }

  void IdenticalSegmentationsHaveNoDifferences()
  {
    AssertNoDifference();

    m_Original = mitk::test::BuildRichSegmentation({ true, false, true, true });
    m_Copy = mitk::test::CloneKeepingUID(*m_Original);
    AssertNoDifference();
  }

  void LabelPropertyValueIsDetected()
  {
    m_Copy->GetLabel(7)->SetOpacity(0.55f);
    AssertDetected("label 7 ('Liver'): property 'opacity'");
  }

  void LostLabelPropertyIsDetected()
  {
    m_Copy->GetLabel(4)->RemoveProperty("tracking_id");
    AssertDetected("label 4 ('Kidney'): property 'tracking_id' expected StringProperty \"TID-4\", actual <absent>");
  }

  void SparseAnatomicRegionIsDetectedWithoutThrowing()
  {
    m_Copy->GetLabel(21)->RemoveAnatomicRegion(0);
    AssertDetected("label 21 ('Spleen'): property 'anatomic_region.[0].value'");
  }

  void MissingAndUnexpectedLabelsAreDetected()
  {
    m_Copy->RemoveLabel(12);
    m_Copy->AddLabel(mitk::Label::New(50, "Extra"), 1, true, false);
    AssertDetected("label 12 ('Quote \" back\\slash {braces}'): missing");
    AssertDetected("label 50 ('Extra'): unexpected");
    AssertDetected("group 0: label values expected [7, 2, 12], actual [7, 2]");
    AssertDetected("group 1: label values expected [30, 4, 21], actual [30, 4, 21, 50]");
  }

  void ColorToleranceIsApplied()
  {
    auto color = m_Copy->GetLabel(30)->GetColor();
    color.SetRed(color.GetRed() + 0.004f);
    m_Copy->GetLabel(30)->SetColor(color);

    AssertDetected("property 'color'");

    mitk::test::SegComparisonOptions options{};
    options.colorTolerance = 0.005f;
    AssertNoDifference(options);
  }

  void GroupNameIsDetected()
  {
    m_Copy->SetGroupName(1, "Renamed");
    AssertDetected("group 1: name expected 'Organs at risk', actual 'Renamed'");
  }

  void VoxelIsDetected()
  {
    // Index 9 of time step 0, row 0, slice 0 lies outside every box of group 0.
    SetVoxel(0, 0, 9, 7);
    AssertDetected("label 7 ('Liver'): 1 of 320 voxels differ in time step 0");
  }

  void VoxelIsDetectedInLaterTimeStep()
  {
    m_Original = mitk::test::BuildRichSegmentation({ true, false, false, false });
    m_Copy = mitk::test::CloneKeepingUID(*m_Original);
    SetVoxel(0, 2, 9, 7);
    AssertDetected("label 7 ('Liver'): 1 of 320 voxels differ in time step 2");
  }

  void StrayValueOutsideLabelsIsDetected()
  {
    // 999 is no label value, so no label mask changes.
    this->SetVoxel(0, 0, 9, 999);
    this->AssertDetected("group 0: 1 of 320 voxels differ in time step 0");
  }

  void EmptyGroupImageIsDetected()
  {
    m_Original = mitk::test::BuildRichSegmentation({ false, false, false, true });
    m_Copy = mitk::test::CloneKeepingUID(*m_Original);
    CPPUNIT_ASSERT_MESSAGE("The fixture's empty group has no labels", m_Original->GetLabelValuesByGroup(2).empty());

    this->SetVoxel(2, 0, 0, 7);
    this->AssertDetected("group 2: 1 of 320 voxels differ in time step 0");
  }

  void SegmentationPropertyIsDetected()
  {
    m_Copy->SetProperty(mitk::test::RichFixtureCustomPropertyName(), mitk::IntProperty::New(43));
    AssertDetected("segmentation: property 'mitk.test.custom' expected IntProperty \"42\", actual IntProperty \"43\"");

    m_Copy->SetProperty("mitk.test.added", mitk::StringProperty::New("x"));
    AssertDetected("segmentation: unexpected property 'mitk.test.added'");
  }

  void PropertyExclusionsAreApplied()
  {
    m_Copy->SetProperty(mitk::test::RichFixtureCustomPropertyName(), mitk::IntProperty::New(43));
    m_Copy->SetProperty("mitk.test.added", mitk::StringProperty::New("x"));
    m_Copy->SetProperty("path", mitk::StringProperty::New("somewhere"));
    m_Copy->SetProperty("MITK.IO.reader.description", mitk::StringProperty::New("reader"));

    mitk::test::SegComparisonOptions options{};
    options.notComparedKeys = { mitk::test::RichFixtureCustomPropertyName() };
    options.allowedAddedKeyPrefixes = { "mitk.test.add" };
    AssertNoDifference(options);

    // An exact key must not hide its namesakes.
    m_Copy->SetProperty(mitk::test::RichFixtureCustomPropertyName() + "ed", mitk::IntProperty::New(1));
    AssertDetected("unexpected property 'mitk.test.customed'", options);

    options.notComparedKeys.clear();
    options.notComparedKeyPrefixes = { "mitk.test.custom" };
    AssertNoDifference(options);
  }

  void UIDIsDetectedUnlessNotCompared()
  {
    m_Copy = m_Original->Clone();
    AssertDetected("segmentation UID");

    mitk::test::SegComparisonOptions options{};
    options.notCompared = { mitk::test::NotCompared::SegmentationUID };
    AssertNoDifference(options);
  }

  void UnlabeledLockIsDetected()
  {
    m_Copy->SetUnlabeledLabelLock(false);
    AssertDetected("unlabeled lock: expected on, actual off");
  }

  void GeometryToleranceIsApplied()
  {
    auto origin = m_Copy->GetGeometry()->GetOrigin();
    origin[1] += 1e-4;
    m_Copy->GetGeometry()->SetOrigin(origin);

    AssertDetected("time step 0: geometry differs");

    mitk::test::SegComparisonOptions options{};
    options.geometryTolerance = 1e-3;
    AssertNoDifference(options);
  }

  void TimeBoundsAreDetected()
  {
    m_Original = mitk::test::BuildRichSegmentation({ true, false, false, false });
    m_Copy = mitk::test::CloneKeepingUID(*m_Original);

    const auto* originalTime = m_Original->GetTimeGeometry();
    auto shifted = mitk::ArbitraryTimeGeometry::New();
    shifted->ClearAllGeometries();
    for (mitk::TimeStepType step = 0; step < originalTime->CountTimeSteps(); ++step)
    {
      const auto delta = step + 1 == originalTime->CountTimeSteps() ? 0.5 : 0.0;
      shifted->AppendNewTimeStepClone(originalTime->GetGeometryForTimeStep(step), originalTime->GetMinimumTimePoint(step),
                                      originalTime->GetMaximumTimePoint(step) + delta);
    }
    m_Copy->SetTimeGeometry(shifted);

    AssertDetected("time step 2: time bounds expected [2.5, 7], actual [2.5, 7.5]");

    mitk::test::SegComparisonOptions options{};
    options.timeTolerance = 0.6;
    AssertNoDifference(options);
  }

  void RelationIsDetected()
  {
    // The factory stamps the same per-slice SOP instance UIDs on every source, so the rule
    // treats this as the existing relation and points it to the other series.
    const auto otherSource = mitk::test::BuildSourceImageWithDICOMIdentity("Other^Source", "OTHER-PID", "OTHER-Study",
      "1.2.826.0.1.3680043.10.999.42.1", "1.2.826.0.1.3680043.10.999.42.2", "1.2.826.0.1.3680043.10.999.42.3",
      mitk::test::detail::RichSizeZ);
    mitk::SegSourceImageRelationRule::Connect(m_Copy, otherSource);
    AssertDetected("actual source series is '1.2.826.0.1.3680043.10.999.42.2'");
  }

  void DocumentedLossIsApplied()
  {
    m_Copy->GetLabel(7)->SetOpacity(0.55f);

    mitk::test::FormatCapabilities capabilities;
    capabilities.formatName = "test format";
    capabilities.losses.push_back({ "opacity of label 7", [](mitk::MultiLabelSegmentation& segmentation) {
      segmentation.GetLabel(7)->SetOpacity(0.55f);
    } });

    mitk::test::AssertReloadMatchesCapabilities({ m_Original.GetPointer() }, { m_Copy.GetPointer() }, capabilities);
  }

  void StaleLossFails()
  {
    mitk::test::FormatCapabilities capabilities;
    capabilities.formatName = "test format";
    capabilities.losses.push_back({ "changes nothing", [](mitk::MultiLabelSegmentation&) {} });

    CPPUNIT_ASSERT_THROW(
      mitk::test::AssertReloadMatchesCapabilities({ m_Original.GetPointer() }, { m_Copy.GetPointer() }, capabilities),
      CppUnit::Exception);
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkSegIOTestComparator)
