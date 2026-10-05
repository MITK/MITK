/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkDICOMFrameLayout.h>

#include <algorithm>

namespace
{
  /** There is deliberately no Error level: with the separation rule of the
      reader, nothing the frame model detects stops a volume from loading, and
      a level with no member would be a schema promise that cannot be kept. */
  mitk::DICOMFrameModelSeverity SeverityOf(mitk::DICOMFrameModelIssue issue)
  {
    switch (issue)
    {
      case mitk::DICOMFrameModelIssue::RaggedFunctionalGroups:
      case mitk::DICOMFrameModelIssue::SharedAndPerFrameRescale:
        return mitk::DICOMFrameModelSeverity::Warning;
      case mitk::DICOMFrameModelIssue::NoPerFrameMetadata:
      case mitk::DICOMFrameModelIssue::VaryingPerFrameRescale:
        break;
    }

    return mitk::DICOMFrameModelSeverity::Info;
  }

  mitk::DICOMFrameModelFinding MakeFinding(mitk::DICOMFrameModelIssue issue,
                                           const mitk::DICOMFrameLayout& layout,
                                           const std::string& filename)
  {
    mitk::DICOMFrameModelFinding finding;
    finding.issue = issue;
    finding.severity = SeverityOf(issue);
    finding.files.push_back(filename);
    finding.frameCount = layout.frameCount;
    finding.perFrameItemCount = layout.perFrameItemCount;
    return finding;
  }

  unsigned int CountDistinct(const std::vector<mitk::DICOMFrameLayout::Rescale>& pairs)
  {
    unsigned int distinct = 0;

    for (auto iter = pairs.cbegin(); iter != pairs.cend(); ++iter)
    {
      const auto earlier = std::find_if(pairs.cbegin(), iter,
                                        [&](const auto& candidate) { return mitk::SameRescale(candidate, *iter); });
      if (earlier == iter)
      {
        ++distinct;
      }
    }

    return distinct;
  }
}

bool mitk::SameRescale(const DICOMFrameLayout::Rescale& left, const DICOMFrameLayout::Rescale& right)
{
  return left.slope == right.slope && left.intercept == right.intercept;
}

std::vector<mitk::DICOMFrameLayout::Rescale> mitk::EffectivePerFrameRescale(const DICOMFrameLayout& layout)
{
  std::vector<DICOMFrameLayout::Rescale> result;
  result.reserve(layout.frameCount);

  for (unsigned int frame = 0; frame < layout.frameCount; ++frame)
  {
    if (frame < layout.perFrameRescale.size() && layout.perFrameRescale[frame].has_value())
    {
      result.push_back(*layout.perFrameRescale[frame]);
    }
    else if (layout.sharedRescale.has_value())
    {
      result.push_back(*layout.sharedRescale);
    }
    else
    {
      result.emplace_back();
    }
  }

  return result;
}

const std::vector<mitk::DICOMFrameModelIssue>& mitk::AllDICOMFrameModelIssues()
{
  static const std::vector<DICOMFrameModelIssue> issues = {
    DICOMFrameModelIssue::RaggedFunctionalGroups,
    DICOMFrameModelIssue::NoPerFrameMetadata,
    DICOMFrameModelIssue::VaryingPerFrameRescale,
    DICOMFrameModelIssue::SharedAndPerFrameRescale
  };

  return issues;
}

std::string mitk::DICOMFrameModelIssueToString(DICOMFrameModelIssue issue)
{
  switch (issue)
  {
    case DICOMFrameModelIssue::RaggedFunctionalGroups:
      return "The Per-Frame Functional Groups Sequence does not have one item per frame; "
             "per-frame values cannot be mapped to slices and the file is read as a single frame.";
    case DICOMFrameModelIssue::NoPerFrameMetadata:
      return "Multi-frame object without per-frame functional groups; per-frame values are not available.";
    case DICOMFrameModelIssue::VaryingPerFrameRescale:
      return "The Pixel Value Transformation differs between frames.";
    case DICOMFrameModelIssue::SharedAndPerFrameRescale:
      return "A shared and a per-frame Pixel Value Transformation are both present; the per-frame one is used.";
  }

  return "Unknown frame model issue.";
}

std::string mitk::DICOMFrameModelIssueToKey(DICOMFrameModelIssue issue)
{
  switch (issue)
  {
    case DICOMFrameModelIssue::RaggedFunctionalGroups:
      return "ragged_functional_groups";
    case DICOMFrameModelIssue::NoPerFrameMetadata:
      return "no_per_frame_metadata";
    case DICOMFrameModelIssue::VaryingPerFrameRescale:
      return "varying_per_frame_rescale";
    case DICOMFrameModelIssue::SharedAndPerFrameRescale:
      return "shared_and_per_frame_rescale";
  }

  return "unknown";
}

std::string mitk::DICOMFrameModelSeverityToKey(DICOMFrameModelSeverity severity)
{
  return DICOMFrameModelSeverity::Warning == severity ? "warning" : "info";
}

std::vector<mitk::DICOMFrameModelFinding> mitk::CollectFrameModelFindings(const DICOMFrameLayout& layout,
                                                                         const std::string& filename)
{
  std::vector<DICOMFrameModelFinding> result;

  if (layout.perFrameItemCount > 0 && layout.perFrameItemCount != layout.frameCount)
  {
    result.push_back(MakeFinding(DICOMFrameModelIssue::RaggedFunctionalGroups, layout, filename));
  }

  if (layout.frameCount > 1 && 0 == layout.perFrameItemCount)
  {
    result.push_back(MakeFinding(DICOMFrameModelIssue::NoPerFrameMetadata, layout, filename));
  }

  // Only a file whose items map one-to-one onto its frames has a per-frame
  // rescale worth comparing. On a ragged sequence EffectivePerFrameRescale pads
  // the unmatched frames from the shared pair or the identity, so comparing them
  // would report a variation the file does not contain.
  if (!layout.perFrameRescale.empty() && layout.HasFrameModel())
  {
    const auto distinct = CountDistinct(EffectivePerFrameRescale(layout));
    if (distinct > 1)
    {
      auto finding = MakeFinding(DICOMFrameModelIssue::VaryingPerFrameRescale, layout, filename);
      finding.distinctRescalePairs = distinct;
      result.push_back(finding);
    }

    if (layout.sharedRescale.has_value())
    {
      result.push_back(MakeFinding(DICOMFrameModelIssue::SharedAndPerFrameRescale, layout, filename));
    }
  }

  return result;
}
