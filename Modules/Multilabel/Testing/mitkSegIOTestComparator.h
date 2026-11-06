/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkSegIOTestComparator_h
#define mitkSegIOTestComparator_h

#include <mitkColorProperty.h>
#include <mitkImageReadAccessor.h>
#include <mitkLabelSetImage.h>
#include <mitkPropertyKeyPath.h>
#include <mitkPropertyRelationRuleBase.h>
#include <mitkSegSourceImageRelationRule.h>
#include <mitkTestingMacros.h>
#include <mitkUIDManipulator.h>

#include <algorithm>
#include <cmath>
#include <functional>
#include <set>
#include <sstream>
#include <string>
#include <vector>

namespace mitk
{
  namespace test
  {
    /** Attributes whose reloaded value cannot be reproduced and are therefore not compared. */
    enum class NotCompared
    {
      SegmentationUID,
      RelationUIDs
    };

    /**
     * Settings that depend on the format under test. Every value in a
     * capability table entry states why it is needed and, for tolerances,
     * how the bound is derived. A default-constructed instance compares
     * everything exactly.
     */
    struct SegComparisonOptions
    {
      /** Maximum absolute difference per colour channel, in [0, 1]. */
      float colorTolerance = 0.f;
      /** Maximum absolute difference of origin, spacing and direction components. */
      ScalarType geometryTolerance = 0.0;
      /** Maximum absolute difference of time points and time bounds. */
      ScalarType timeTolerance = 0.0;
      std::set<NotCompared> notCompared;
      /**
       * Property keys excluded on both sides because their reloaded value
       * cannot be reproduced. Exact keys and prefix families are kept apart
       * so that excluding one key cannot hide its namesakes.
       */
      std::set<std::string> notComparedKeys;
      std::vector<std::string> notComparedKeyPrefixes;
      /** Key prefixes the reader may add that the expected side does not have. */
      std::vector<std::string> allowedAddedKeyPrefixes;
    };

    namespace detail
    {
      inline bool StartsWith(const std::string& text, const std::string& prefix)
      {
        return text.compare(0, prefix.size(), prefix) == 0;
      }

      inline bool StartsWithAny(const std::string& text, const std::vector<std::string>& prefixes)
      {
        return std::any_of(prefixes.begin(), prefixes.end(),
          [&text](const std::string& prefix) { return StartsWith(text, prefix); });
      }

      inline std::string DescribeLabel(const Label* label)
      {
        std::ostringstream stream;
        stream << "label " << label->GetValue() << " ('" << label->GetName() << "')";
        return stream.str();
      }

      inline std::string DescribeProperty(const BaseProperty* property)
      {
        if (nullptr == property)
          return "<absent>";
        return std::string(property->GetNameOfClass()) + " \"" + property->GetValueAsString() + "\"";
      }

      inline bool PropertiesEqual(const BaseProperty* expected, const BaseProperty* actual, float colorTolerance)
      {
        const auto* expectedColor = dynamic_cast<const ColorProperty*>(expected);
        const auto* actualColor = dynamic_cast<const ColorProperty*>(actual);
        if (nullptr != expectedColor && nullptr != actualColor)
        {
          for (unsigned int channel = 0; channel < 3; ++channel)
          {
            if (std::abs(expectedColor->GetColor()[channel] - actualColor->GetColor()[channel]) > colorTolerance)
              return false;
          }
          return true;
        }
        return *expected == *actual;
      }

      /** Compares two property maps key by key; keys rejected by isIgnored are skipped on both sides. */
      inline void CompareProperties(const PropertyList::PropertyMap& expected,
                                    const PropertyList::PropertyMap& actual,
                                    const std::function<bool(const std::string&)>& isIgnored,
                                    const std::vector<std::string>& allowedAddedKeyPrefixes,
                                    float colorTolerance,
                                    const std::string& context,
                                    std::vector<std::string>& differences)
      {
        for (const auto& [key, expectedProperty] : expected)
        {
          if (isIgnored(key))
            continue;

          const auto finding = actual.find(key);
          const BaseProperty* actualProperty = finding == actual.end() ? nullptr : finding->second.GetPointer();
          if (nullptr == actualProperty || !PropertiesEqual(expectedProperty, actualProperty, colorTolerance))
          {
            differences.push_back(context + ": property '" + key + "' expected " + DescribeProperty(expectedProperty) +
                                  ", actual " + DescribeProperty(actualProperty));
          }
        }

        for (const auto& [key, actualProperty] : actual)
        {
          if (isIgnored(key) || expected.find(key) != expected.end() || StartsWithAny(key, allowedAddedKeyPrefixes))
            continue;
          differences.push_back(context + ": unexpected property '" + key + "' " + DescribeProperty(actualProperty));
        }
      }

      inline void CompareLabels(const MultiLabelSegmentation& expected,
                                const MultiLabelSegmentation& actual,
                                const SegComparisonOptions& options,
                                std::vector<std::string>& differences)
      {
        // The JSON writer drops the centre of mass; it is recomputed from the voxels on demand.
        const auto isIgnored = [](const std::string& key) { return StartsWith(key, "center."); };

        for (const auto value : expected.GetAllLabelValues())
        {
          const auto expectedLabel = expected.GetLabel(value);
          if (!actual.ExistLabel(value))
          {
            differences.push_back(DescribeLabel(expectedLabel) + ": missing");
            continue;
          }
          const auto actualLabel = actual.GetLabel(value);
          CompareProperties(*expectedLabel->GetMap(), *actualLabel->GetMap(), isIgnored, {},
                            options.colorTolerance, DescribeLabel(expectedLabel), differences);
        }

        for (const auto value : actual.GetAllLabelValues())
        {
          if (!expected.ExistLabel(value))
            differences.push_back(DescribeLabel(actual.GetLabel(value)) + ": unexpected");
        }
      }

      inline std::string DescribeValues(const MultiLabelSegmentation::LabelValueVectorType& values)
      {
        std::ostringstream stream;
        stream << "[";
        for (std::size_t i = 0; i < values.size(); ++i)
          stream << (i == 0 ? "" : ", ") << values[i];
        stream << "]";
        return stream.str();
      }

      inline void CompareGroups(const MultiLabelSegmentation& expected,
                                const MultiLabelSegmentation& actual,
                                std::vector<std::string>& differences)
      {
        if (expected.GetNumberOfGroups() != actual.GetNumberOfGroups())
        {
          differences.push_back("group count: expected " + std::to_string(expected.GetNumberOfGroups()) +
                                ", actual " + std::to_string(actual.GetNumberOfGroups()));
        }

        const auto groupCount = std::min(expected.GetNumberOfGroups(), actual.GetNumberOfGroups());
        for (MultiLabelSegmentation::GroupIndexType group = 0; group < groupCount; ++group)
        {
          const auto context = "group " + std::to_string(group);
          if (expected.GetGroupName(group) != actual.GetGroupName(group))
          {
            differences.push_back(context + ": name expected '" + expected.GetGroupName(group) + "', actual '" +
                                  actual.GetGroupName(group) + "'");
          }

          const auto expectedValues = expected.GetLabelValuesByGroup(group);
          const auto actualValues = actual.GetLabelValuesByGroup(group);
          if (expectedValues != actualValues)
          {
            differences.push_back(context + ": label values expected " + DescribeValues(expectedValues) +
                                  ", actual " + DescribeValues(actualValues));
          }
        }
      }

      /**
       * Compares every group image voxel by voxel in every time step, so
       * values that belong to no label count as well. A differing time step
       * is also broken down per label the group has on both sides, as masks:
       * a voxel matches when it carries the label on both sides or on neither.
       */
      inline void CompareVoxels(const MultiLabelSegmentation& expected,
                                const MultiLabelSegmentation& actual,
                                std::vector<std::string>& differences)
      {
        const auto labelPixelType = MakeScalarPixelType<Label::PixelType>();

        const auto groupCount = std::min(expected.GetNumberOfGroups(), actual.GetNumberOfGroups());
        for (MultiLabelSegmentation::GroupIndexType group = 0; group < groupCount; ++group)
        {
          const auto context = "group " + std::to_string(group);
          const auto* expectedImage = expected.GetGroupImage(group);
          const auto* actualImage = actual.GetGroupImage(group);

          if (expectedImage->GetPixelType() != labelPixelType || actualImage->GetPixelType() != labelPixelType)
          {
            differences.push_back(context + ": image pixel type differs from the label pixel type");
            continue;
          }

          bool sameShape = expectedImage->GetDimension() == actualImage->GetDimension();
          for (unsigned int axis = 0; sameShape && axis < expectedImage->GetDimension(); ++axis)
            sameShape = expectedImage->GetDimension(axis) == actualImage->GetDimension(axis);
          if (!sameShape)
          {
            differences.push_back(context + ": images differ in shape");
            continue;
          }

          const auto actualValues = actual.GetLabelValuesByGroup(group);
          MultiLabelSegmentation::LabelValueVectorType sharedValues;
          for (const auto value : expected.GetLabelValuesByGroup(group))
          {
            if (std::find(actualValues.begin(), actualValues.end(), value) != actualValues.end())
              sharedValues.push_back(value);
          }

          const std::size_t voxelsPerTimeStep =
            static_cast<std::size_t>(expectedImage->GetDimension(0)) * expectedImage->GetDimension(1) * expectedImage->GetDimension(2);
          const auto timeSteps = expectedImage->GetTimeSteps();

          ImageReadAccessor expectedAccessor(expectedImage);
          ImageReadAccessor actualAccessor(actualImage);
          const auto* expectedVoxels = static_cast<const Label::PixelType*>(expectedAccessor.GetData());
          const auto* actualVoxels = static_cast<const Label::PixelType*>(actualAccessor.GetData());

          for (unsigned int timeStep = 0; timeStep < timeSteps; ++timeStep)
          {
            const auto* expectedBegin = expectedVoxels + timeStep * voxelsPerTimeStep;
            const auto* actualBegin = actualVoxels + timeStep * voxelsPerTimeStep;
            if (std::equal(expectedBegin, expectedBegin + voxelsPerTimeStep, actualBegin))
              continue;

            const auto ofVoxels = " of " + std::to_string(voxelsPerTimeStep) + " voxels differ in time step " +
                                  std::to_string(timeStep);
            std::size_t mismatches = 0;
            for (std::size_t i = 0; i < voxelsPerTimeStep; ++i)
            {
              if (expectedBegin[i] != actualBegin[i])
                ++mismatches;
            }
            differences.push_back(context + ": " + std::to_string(mismatches) + ofVoxels);

            // The breakdown costs one pass per label, so equal time steps skip it.
            for (const auto value : sharedValues)
            {
              std::size_t labelMismatches = 0;
              for (std::size_t i = 0; i < voxelsPerTimeStep; ++i)
              {
                if ((expectedBegin[i] == value) != (actualBegin[i] == value))
                  ++labelMismatches;
              }
              if (labelMismatches > 0)
                differences.push_back(DescribeLabel(expected.GetLabel(value)) + ": " + std::to_string(labelMismatches) + ofVoxels);
            }
          }
        }
      }

      inline bool TimePointsMatch(TimePointType expected, TimePointType actual, ScalarType tolerance)
      {
        return expected == actual || std::abs(expected - actual) <= tolerance;
      }

      inline void CompareGeometry(const MultiLabelSegmentation& expected,
                                  const MultiLabelSegmentation& actual,
                                  const SegComparisonOptions& options,
                                  std::vector<std::string>& differences)
      {
        const auto* expectedTime = expected.GetTimeGeometry();
        const auto* actualTime = actual.GetTimeGeometry();
        // mitk::Equal treats a difference equal to its tolerance as unequal, so a tolerance of 0
        // would reject identical geometries.
        const auto geometryTolerance = std::max(options.geometryTolerance, mitk::eps);

        if (expectedTime->CountTimeSteps() != actualTime->CountTimeSteps())
        {
          differences.push_back("time steps: expected " + std::to_string(expectedTime->CountTimeSteps()) +
                                ", actual " + std::to_string(actualTime->CountTimeSteps()));
          return;
        }

        for (TimeStepType timeStep = 0; timeStep < expectedTime->CountTimeSteps(); ++timeStep)
        {
          const auto context = "time step " + std::to_string(timeStep);
          const auto expectedGeometry = expectedTime->GetGeometryForTimeStep(timeStep);
          const auto actualGeometry = actualTime->GetGeometryForTimeStep(timeStep);

          if (!Equal(*expectedGeometry, *actualGeometry, geometryTolerance, geometryTolerance, false))
          {
            std::ostringstream stream;
            stream << context << ": geometry differs; origin expected " << expectedGeometry->GetOrigin() << ", actual "
                   << actualGeometry->GetOrigin() << "; spacing expected " << expectedGeometry->GetSpacing()
                   << ", actual " << actualGeometry->GetSpacing();
            differences.push_back(stream.str());
          }

          const auto expectedMin = expectedTime->GetMinimumTimePoint(timeStep);
          const auto actualMin = actualTime->GetMinimumTimePoint(timeStep);
          const auto expectedMax = expectedTime->GetMaximumTimePoint(timeStep);
          const auto actualMax = actualTime->GetMaximumTimePoint(timeStep);
          if (!TimePointsMatch(expectedMin, actualMin, options.timeTolerance) ||
              !TimePointsMatch(expectedMax, actualMax, options.timeTolerance))
          {
            std::ostringstream stream;
            stream << context << ": time bounds expected [" << expectedMin << ", " << expectedMax << "], actual ["
                   << actualMin << ", " << actualMax << "]";
            differences.push_back(stream.str());
          }
        }
      }

      inline void CompareRelations(const MultiLabelSegmentation& expected,
                                   const MultiLabelSegmentation& actual,
                                   const SegComparisonOptions& options,
                                   std::vector<std::string>& differences)
      {
        auto expectedRelations = SegSourceImageRelationRule::GetSourceImageRelations(&expected);
        auto actualRelations = SegSourceImageRelationRule::GetSourceImageRelations(&actual);

        if (expectedRelations.size() != actualRelations.size())
        {
          differences.push_back("source-image relations: expected " + std::to_string(expectedRelations.size()) +
                                ", actual " + std::to_string(actualRelations.size()));
          return;
        }

        const bool compareRelationUIDs = options.notCompared.count(NotCompared::RelationUIDs) == 0;
        const auto byKey = [compareRelationUIDs](const SegSourceImageRelationRule::SourceImageRelation& lhs,
                                                 const SegSourceImageRelationRule::SourceImageRelation& rhs) {
          return compareRelationUIDs ? lhs.relationUID < rhs.relationUID
                                     : lhs.sourceSeriesInstanceUID < rhs.sourceSeriesInstanceUID;
        };
        std::sort(expectedRelations.begin(), expectedRelations.end(), byKey);
        std::sort(actualRelations.begin(), actualRelations.end(), byKey);

        const auto samePerSliceProperty = [](const TemporoSpatialStringProperty* lhs, const TemporoSpatialStringProperty* rhs) {
          if (nullptr == lhs || nullptr == rhs)
            return lhs == rhs;
          return *lhs == *rhs;
        };

        for (std::size_t i = 0; i < expectedRelations.size(); ++i)
        {
          const auto& expectedRelation = expectedRelations[i];
          const auto& actualRelation = actualRelations[i];
          const auto context = "source-image relation to series '" + expectedRelation.sourceSeriesInstanceUID + "'";

          if (compareRelationUIDs && expectedRelation.relationUID != actualRelation.relationUID)
          {
            differences.push_back(context + ": relation UID expected '" + expectedRelation.relationUID +
                                  "', actual '" + actualRelation.relationUID + "'");
          }
          if (expectedRelation.sourceSeriesInstanceUID != actualRelation.sourceSeriesInstanceUID)
          {
            differences.push_back(context + ": actual source series is '" + actualRelation.sourceSeriesInstanceUID + "'");
          }
          if (!samePerSliceProperty(expectedRelation.instanceUIDsPerSlice, actualRelation.instanceUIDsPerSlice))
          {
            differences.push_back(context + ": per-slice SOP instance UIDs differ");
          }
          if (!samePerSliceProperty(expectedRelation.classUIDsPerSlice, actualRelation.classUIDsPerSlice))
          {
            differences.push_back(context + ": per-slice SOP class UIDs differ");
          }
        }
      }

      inline void CompareSegmentationProperties(const MultiLabelSegmentation& expected,
                                                const MultiLabelSegmentation& actual,
                                                const SegComparisonOptions& options,
                                                std::vector<std::string>& differences)
      {
        // Relations are compared through the relation rule, so their storage keys stay out of
        // the generic comparison. "path" and "MITK.IO." are stamped by IOUtil and the readers.
        const auto relationRoot = PropertyKeyPathToPropertyName(PropertyRelationRuleBase::GetRootKeyPath()) + ".";
        const std::vector<std::string> ignoredPrefixes = { relationRoot, "DICOM.0008.2112.", "MITK.IO." };

        const auto isIgnored = [&options, &ignoredPrefixes](const std::string& key) {
          return key == "path" || StartsWithAny(key, ignoredPrefixes) || options.notComparedKeys.count(key) > 0 ||
                 StartsWithAny(key, options.notComparedKeyPrefixes);
        };

        CompareProperties(*expected.GetPropertyList()->GetMap(), *actual.GetPropertyList()->GetMap(), isIgnored,
                          options.allowedAddedKeyPrefixes, options.colorTolerance, "segmentation", differences);
      }
    }

    /**
     * Compares two segmentations attribute by attribute and returns one
     * readable line per difference; an empty result means equal. Labels
     * are matched by value.
     *
     * Per label: every property by key, type and value. That covers name,
     * colour (within the colour tolerance), opacity, visible, locked,
     * description, tracking ID and UID, algorithm type and name, the
     * segmented property category and type with all modifiers, all
     * anatomic regions and primary anatomic structures with all modifiers,
     * and custom label properties; presence is compared as well, so a lost
     * tracking ID or algorithm name shows up even where the getter would
     * fall back to a default.
     * Per group: name, member label values in order, and every voxel in
     * every time step, with differing time steps broken down per label.
     * Per segmentation: UID, unlabeled lock, the property list by key, type
     * and value, source-image relations, the geometry of every time step,
     * and the time bounds of every time step.
     * The key "path" and the prefix "MITK.IO." are never compared: IOUtil
     * and the readers stamp them on load.
     * The active label is not compared; no format promises to keep it.
     */
    inline std::vector<std::string> CompareSegmentations(const MultiLabelSegmentation& expected,
                                                         const MultiLabelSegmentation& actual,
                                                         const SegComparisonOptions& options)
    {
      std::vector<std::string> differences;

      if (options.notCompared.count(NotCompared::SegmentationUID) == 0 && expected.GetUID() != actual.GetUID())
        differences.push_back("segmentation UID: expected '" + expected.GetUID() + "', actual '" + actual.GetUID() + "'");

      if (expected.GetUnlabeledLabelLock() != actual.GetUnlabeledLabelLock())
      {
        differences.push_back(std::string("unlabeled lock: expected ") + (expected.GetUnlabeledLabelLock() ? "on" : "off") +
                              ", actual " + (actual.GetUnlabeledLabelLock() ? "on" : "off"));
      }

      detail::CompareGeometry(expected, actual, options, differences);
      detail::CompareGroups(expected, actual, differences);
      detail::CompareLabels(expected, actual, options, differences);
      detail::CompareVoxels(expected, actual, differences);
      detail::CompareSegmentationProperties(expected, actual, options, differences);
      detail::CompareRelations(expected, actual, options, differences);

      return differences;
    }

    /** Clones a segmentation; Clone() alone mints a new UID. */
    inline MultiLabelSegmentation::Pointer CloneKeepingUID(const MultiLabelSegmentation& segmentation)
    {
      auto clone = segmentation.Clone();
      UIDManipulator uidManipulator(clone);
      uidManipulator.SetUID(segmentation.GetUID());
      return clone;
    }

    /** One documented loss: why it happens, and the edit that reproduces it on the expected side. */
    struct DocumentedLoss
    {
      std::string description;
      std::function<void(MultiLabelSegmentation&)> apply;
    };

    struct FormatCapabilities
    {
      std::string formatName;
      SegComparisonOptions options;
      std::vector<DocumentedLoss> losses;
    };

    inline std::string JoinDifferences(const std::vector<std::string>& differences)
    {
      std::ostringstream stream;
      for (const auto& difference : differences)
        stream << "\n  - " << difference;
      return stream.str();
    }

    /**
     * One write can yield several reloaded segmentations (DICOM SEG writes
     * one file per group); originals[i] is the expectation for reloaded[i].
     * Applies every loss to a clone of every original and asserts that each
     * loss changes at least one of the clones (a loss that changes nothing
     * anywhere is stale, or the fixture does not exercise it). Then asserts
     * that CompareSegmentations(clone[i], reloaded[i]) is empty for every i.
     */
    inline void AssertReloadMatchesCapabilities(const std::vector<MultiLabelSegmentation::ConstPointer>& originals,
                                                const std::vector<MultiLabelSegmentation::ConstPointer>& reloaded,
                                                const FormatCapabilities& capabilities)
    {
      CPPUNIT_ASSERT_EQUAL_MESSAGE(capabilities.formatName + ": number of reloaded segmentations",
                                   originals.size(), reloaded.size());

      std::vector<MultiLabelSegmentation::Pointer> expectations;
      for (const auto& original : originals)
        expectations.push_back(CloneKeepingUID(*original));

      for (const auto& loss : capabilities.losses)
      {
        bool changedAny = false;
        for (auto& expectation : expectations)
        {
          const auto before = CloneKeepingUID(*expectation);
          loss.apply(*expectation);
          changedAny = changedAny || !CompareSegmentations(*before, *expectation, SegComparisonOptions{}).empty();
        }
        CPPUNIT_ASSERT_MESSAGE(capabilities.formatName + ": the documented loss \"" + loss.description +
                                 "\" changes nothing in the fixture; the entry is stale or the fixture does not exercise it",
                               changedAny);
      }

      for (std::size_t i = 0; i < expectations.size(); ++i)
      {
        const auto differences = CompareSegmentations(*expectations[i], *reloaded[i], capabilities.options);
        if (!differences.empty())
        {
          CPPUNIT_FAIL(capabilities.formatName + ", reloaded segmentation " + std::to_string(i) +
                       " differs from the original after the documented losses. A difference on an attribute a"
                       " loss edits means the loss entry is stale." + JoinDifferences(differences));
        }
      }
    }
  }
}

#endif
