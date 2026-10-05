/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkDICOMFrameLayout.h>

#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <algorithm>

/**
 * \brief Covers the frame layout predicates and the findings derived from them.
 *
 * These decide whether a multi-frame file gets the per-frame read model at all,
 * and what the reader logs and the diagnostics CLI reports about it, so they are
 * tested on the layout directly rather than through a generated file.
 */
class mitkDICOMFrameLayoutTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkDICOMFrameLayoutTestSuite);

  MITK_TEST(HasFrameModelRequiresOneItemPerFrame);
  MITK_TEST(EffectiveRescaleFallsBackFromFrameToSharedToIdentity);
  MITK_TEST(ConformantLayoutHasNoFindings);
  MITK_TEST(RaggedSequenceIsAWarning);
  MITK_TEST(RaggedSequenceDoesNotInventAVaryingRescale);
  MITK_TEST(PlainMultiFrameIsReportedAsInfo);
  MITK_TEST(VaryingRescaleCountsItsDistinctPairs);
  MITK_TEST(SharedAndPerFrameRescaleIsAWarning);
  MITK_TEST(KeysAreStableSnakeCaseForEveryEnumerator);

  CPPUNIT_TEST_SUITE_END();

private:
  /** A layout with one per-frame item per frame, each with its own pair. */
  static mitk::DICOMFrameLayout WithPerFrameRescale(const std::vector<double>& slopes)
  {
    mitk::DICOMFrameLayout layout;
    layout.frameCount = static_cast<unsigned int>(slopes.size());
    layout.perFrameItemCount = layout.frameCount;

    for (const auto slope : slopes)
    {
      mitk::DICOMFrameLayout::Rescale rescale;
      rescale.slope = slope;
      layout.perFrameRescale.push_back(rescale);
    }

    return layout;
  }

  static bool Has(const std::vector<mitk::DICOMFrameModelFinding>& findings, mitk::DICOMFrameModelIssue issue)
  {
    return findings.cend() != std::find_if(findings.cbegin(), findings.cend(),
                                           [issue](const auto& finding) { return finding.issue == issue; });
  }

  static mitk::DICOMFrameModelFinding The(const std::vector<mitk::DICOMFrameModelFinding>& findings,
                                          mitk::DICOMFrameModelIssue issue)
  {
    const auto finding = std::find_if(findings.cbegin(), findings.cend(),
                                      [issue](const auto& candidate) { return candidate.issue == issue; });
    CPPUNIT_ASSERT_MESSAGE("The expected finding is present", findings.cend() != finding);
    return *finding;
  }

public:
  void HasFrameModelRequiresOneItemPerFrame()
  {
    mitk::DICOMFrameLayout layout;
    CPPUNIT_ASSERT_MESSAGE("A default layout is the one-frame model without groups", !layout.HasFrameModel());

    layout.frameCount = 20;
    layout.perFrameItemCount = 20;
    CPPUNIT_ASSERT_MESSAGE("One item per frame is the frame model", layout.HasFrameModel());

    layout.perFrameItemCount = 19;
    CPPUNIT_ASSERT_MESSAGE("A sequence that misses a frame cannot be mapped to slots",
                           !layout.HasFrameModel());

    layout.perFrameItemCount = 21;
    CPPUNIT_ASSERT_MESSAGE("A sequence with a surplus item cannot be mapped either",
                           !layout.HasFrameModel());

    // The single-frame functional-group object, which does get the frame model
    // so that every such object presents its values the same way.
    layout.frameCount = 1;
    layout.perFrameItemCount = 1;
    CPPUNIT_ASSERT_MESSAGE("A single-frame functional-group object has the frame model",
                           layout.HasFrameModel());
  }

  void EffectiveRescaleFallsBackFromFrameToSharedToIdentity()
  {
    mitk::DICOMFrameLayout layout;
    layout.frameCount = 3;
    layout.perFrameItemCount = 3;

    mitk::DICOMFrameLayout::Rescale shared;
    shared.slope = 2.0;
    shared.intercept = 5.0;
    layout.sharedRescale = shared;

    mitk::DICOMFrameLayout::Rescale own;
    own.slope = 7.0;
    layout.perFrameRescale = { own, std::nullopt, std::nullopt };

    const auto effective = mitk::EffectivePerFrameRescale(layout);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("One entry per frame", std::size_t(3), effective.size());
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("The frame's own pair wins", 7.0, effective[0].slope, 1e-12);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("The shared pair fills a frame without one",
                                         2.0, effective[1].slope, 1e-12);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("The shared intercept comes with it",
                                         5.0, effective[2].intercept, 1e-12);

    layout.sharedRescale.reset();
    layout.perFrameRescale = { std::nullopt, std::nullopt, std::nullopt };
    const auto identity = mitk::EffectivePerFrameRescale(layout);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Neither present means the identity", 1.0, identity[1].slope, 1e-12);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Neither present means the identity", 0.0, identity[1].intercept, 1e-12);
  }

  void ConformantLayoutHasNoFindings()
  {
    const auto layout = WithPerFrameRescale({ 4.0, 4.0, 4.0 });
    CPPUNIT_ASSERT_MESSAGE("A uniform functional-group object has nothing to report",
                           mitk::CollectFrameModelFindings(layout, "conformant.dcm").empty());
  }

  void RaggedSequenceIsAWarning()
  {
    mitk::DICOMFrameLayout layout;
    layout.frameCount = 20;
    layout.perFrameItemCount = 19;

    const auto findings = mitk::CollectFrameModelFindings(layout, "ragged.dcm");
    const auto finding = The(findings, mitk::DICOMFrameModelIssue::RaggedFunctionalGroups);

    CPPUNIT_ASSERT_MESSAGE("A ragged sequence is worth a log line",
                           mitk::DICOMFrameModelSeverity::Warning == finding.severity);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("The finding carries the frame count", 20u, finding.frameCount);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("The finding carries the item count", 19u, finding.perFrameItemCount);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("The finding names its file", std::string("ragged.dcm"), finding.files.front());
    CPPUNIT_ASSERT_MESSAGE("A ragged sequence is not also reported as missing metadata",
                           !Has(findings, mitk::DICOMFrameModelIssue::NoPerFrameMetadata));
  }

  /**
   * The per-frame rescale of a ragged file describes its items, not its frames,
   * so the frames beyond the last item have no pair of their own. Padding them
   * from the shared pair or the identity is what the correction needs, but it
   * must not be read back as a variation the file does not contain.
   */
  void RaggedSequenceDoesNotInventAVaryingRescale()
  {
    mitk::DICOMFrameLayout layout;
    layout.frameCount = 20;
    layout.perFrameItemCount = 19;
    layout.perFrameRescale.assign(19, mitk::DICOMFrameLayout::Rescale{ 2.5, 0.0 });

    const auto findings = mitk::CollectFrameModelFindings(layout, "ragged.dcm");

    CPPUNIT_ASSERT_MESSAGE("The ragged sequence is still reported",
                           Has(findings, mitk::DICOMFrameModelIssue::RaggedFunctionalGroups));
    CPPUNIT_ASSERT_MESSAGE("Uniform items are not reported as a varying rescale",
                           !Has(findings, mitk::DICOMFrameModelIssue::VaryingPerFrameRescale));
  }

  void PlainMultiFrameIsReportedAsInfo()
  {
    mitk::DICOMFrameLayout layout;
    layout.frameCount = 263;

    const auto finding = The(mitk::CollectFrameModelFindings(layout, "RD.dcm"),
                             mitk::DICOMFrameModelIssue::NoPerFrameMetadata);

    // The normal state of RT dose, NM, SC and US: reported as data by the
    // diagnostics CLI, never as a log line on every load.
    CPPUNIT_ASSERT_MESSAGE("A plain multi-frame object is informational, not a warning",
                           mitk::DICOMFrameModelSeverity::Info == finding.severity);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("The finding carries the frame count", 263u, finding.frameCount);

    mitk::DICOMFrameLayout singleFrame;
    CPPUNIT_ASSERT_MESSAGE("A single-frame file reports nothing",
                           mitk::CollectFrameModelFindings(singleFrame, "slice.dcm").empty());
  }

  void VaryingRescaleCountsItsDistinctPairs()
  {
    const auto layout = WithPerFrameRescale({ 4.0, 4.0, 3.0, 4.0, 3.0 });
    const auto finding = The(mitk::CollectFrameModelFindings(layout, "DRO_7_1_0.dcm"),
                             mitk::DICOMFrameModelIssue::VaryingPerFrameRescale);

    CPPUNIT_ASSERT_MESSAGE("Varying rescale is informational; the reader corrects it",
                           mitk::DICOMFrameModelSeverity::Info == finding.severity);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Two distinct pairs, however often each occurs",
                                 2u, finding.distinctRescalePairs);
  }

  void SharedAndPerFrameRescaleIsAWarning()
  {
    auto layout = WithPerFrameRescale({ 4.0, 4.0 });
    mitk::DICOMFrameLayout::Rescale shared;
    shared.slope = 2.0;
    layout.sharedRescale = shared;

    const auto findings = mitk::CollectFrameModelFindings(layout, "nonconformant.dcm");
    const auto finding = The(findings, mitk::DICOMFrameModelIssue::SharedAndPerFrameRescale);

    CPPUNIT_ASSERT_MESSAGE("Both placements in one file is worth a log line",
                           mitk::DICOMFrameModelSeverity::Warning == finding.severity);
    CPPUNIT_ASSERT_MESSAGE("The per-frame pairs agree, so nothing varies",
                           !Has(findings, mitk::DICOMFrameModelIssue::VaryingPerFrameRescale));
  }

  /** The keys are the machine-readable half of the diagnostics report, so they
      are a contract: a message may be reworded, a key may not. */
  void KeysAreStableSnakeCaseForEveryEnumerator()
  {
    const std::vector<std::pair<mitk::DICOMFrameModelIssue, std::string>> expected{
      { mitk::DICOMFrameModelIssue::RaggedFunctionalGroups, "ragged_functional_groups" },
      { mitk::DICOMFrameModelIssue::NoPerFrameMetadata, "no_per_frame_metadata" },
      { mitk::DICOMFrameModelIssue::VaryingPerFrameRescale, "varying_per_frame_rescale" },
      { mitk::DICOMFrameModelIssue::SharedAndPerFrameRescale, "shared_and_per_frame_rescale" }
    };

    for (const auto& [issue, key] : expected)
    {
      CPPUNIT_ASSERT_EQUAL_MESSAGE("Stable key", key, mitk::DICOMFrameModelIssueToKey(issue));
      CPPUNIT_ASSERT_MESSAGE("Every issue has a message", !mitk::DICOMFrameModelIssueToString(issue).empty());
    }

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Stable severity key", std::string("warning"),
                                 mitk::DICOMFrameModelSeverityToKey(mitk::DICOMFrameModelSeverity::Warning));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Stable severity key", std::string("info"),
                                 mitk::DICOMFrameModelSeverityToKey(mitk::DICOMFrameModelSeverity::Info));
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkDICOMFrameLayout)
