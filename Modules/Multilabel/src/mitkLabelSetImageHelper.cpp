/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkLabelSetImageHelper.h>

#include <mitkDataStorage.h>
#include <mitkDICOMSegmentationPropertyHelper.h>
#include <mitkLabelSetImage.h>
#include <mitkExceptionMacro.h>
#include <mitkLog.h>
#include <mitkProperties.h>
#include <mitkSegSourceImageRelationRule.h>

#include <vtkMath.h>

#include <array>
#include <limits>
#include <regex>
#include <vector>

namespace
{
  // Convert an RGB triple in [0,1] to CIE Lab via VTK. L is in [0, 100];
  // a and b are roughly in [-110, 110].
  std::array<double, 3> RGBToLab(const double* rgb)
  {
    std::array<double, 3> lab{};
    vtkMath::RGBToLab(rgb, lab.data());
    return lab;
  }

  std::array<double, 3> RGBToLab(const mitk::Color& color)
  {
    const double rgb[3] = {
      static_cast<double>(color.GetRed()),
      static_cast<double>(color.GetGreen()),
      static_cast<double>(color.GetBlue()) };
    return RGBToLab(rgb);
  }

  // Squared Euclidean distance in Lab (ΔE76 squared). We only compare
  // distances, so we never need the sqrt.
  double DeltaE76Squared(const std::array<double, 3>& a, const std::array<double, 3>& b)
  {
    const double dL = a[0] - b[0];
    const double da = a[1] - b[1];
    const double db = a[2] - b[2];
    return dL * dL + da * da + db * db;
  }

  // Distance (squared ΔE76) at which the curated palette counts as
  // exhausted: a candidate nearer than this to every in-use color can no
  // longer be a distinct palette color, so generated extras take over. The
  // bound is the palette's own minimum pairwise distance, so while any
  // palette color is unused its best candidate still clears it; only once
  // all sit too close to the in-use colors do we fall through to extras. It
  // must be perceptual, not a tiny epsilon: a user can nudge a label close
  // to (not exactly onto) a palette color, and we then want to pass that
  // palette color over rather than reuse a near-duplicate. The palette is
  // constant, so this is evaluated once.
  double ComputePaletteExhaustedThresholdSquared()
  {
    const int paletteColorCount = mitk::LookupTable::GetMultiLabelColorCount();

    std::vector<std::array<double, 3>> paletteLab;
    paletteLab.reserve(paletteColorCount);
    for (int i = 0; i < paletteColorCount; ++i)
    {
      std::array<double, 3> rgb{};
      mitk::LookupTable::GetMultiLabelColor(i, rgb.data());
      paletteLab.push_back(RGBToLab(rgb.data()));
    }

    double minSquared = std::numeric_limits<double>::infinity();
    for (size_t a = 0; a < paletteLab.size(); ++a)
      for (size_t b = a + 1; b < paletteLab.size(); ++b)
      {
        const double d2 = DeltaE76Squared(paletteLab[a], paletteLab[b]);
        if (d2 < minSquared)
          minSquared = d2;
      }

    // A hair below the exact minimum: in-use label colors are stored as
    // float while candidates are evaluated at double precision, and that
    // rounding must not let a still-unused palette color read as exhausted.
    return minSquared * 0.999;
  }
}

mitk::DataNode::Pointer mitk::LabelSetImageHelper::CreateEmptySegmentationNode(const std::string& segmentationName)
{
  auto newSegmentationNode = mitk::DataNode::New();
  newSegmentationNode->SetName(segmentationName);

  // initialize "showVolume"-property to false to prevent recalculating the volume while working on the segmentation
  newSegmentationNode->SetProperty("showVolume", mitk::BoolProperty::New(false));

  return newSegmentationNode;
}


mitk::DataNode::Pointer mitk::LabelSetImageHelper::CreateNewSegmentationNode(const DataNode* referenceNode,
  const Image* initialSegmentationImage, const std::string& segmentationName, const DataStorage* dataStorage)
{
  std::string newSegmentationName = segmentationName;
  if (newSegmentationName.empty())
  {
    newSegmentationName = (nullptr!= referenceNode)? referenceNode->GetName() : "unknown";

    if (!newSegmentationName.empty())
      newSegmentationName.append("-");

    newSegmentationName.append("labels");
  }

  if (dataStorage != nullptr && dataStorage->GetNamedNode(newSegmentationName) != nullptr)
  {
    int id = 2;
    std::string suffix = "-" + std::to_string(id);

    while (dataStorage->GetNamedNode(newSegmentationName + suffix) != nullptr)
      suffix = "-" + std::to_string(++id);

    newSegmentationName.append(suffix);
  }

  if (nullptr == initialSegmentationImage)
  {
    return nullptr;
  }

  auto newLabelSetImage = mitk::MultiLabelSegmentation::New();
  try
  {
    newLabelSetImage->Initialize(initialSegmentationImage);
  }
  catch (mitk::Exception &e)
  {
    mitkReThrow(e) << "Could not initialize new label set image.";
    return nullptr;
  }

  auto newSegmentationNode = CreateEmptySegmentationNode(newSegmentationName);
  newSegmentationNode->SetData(newLabelSetImage);

  if (referenceNode != nullptr)
  {
    if (auto referenceImage = dynamic_cast<const Image*>(referenceNode->GetData()))
    {
      SetupDerivedSegmentation(newLabelSetImage, referenceImage);
    }
  }

  return newSegmentationNode;
}

void mitk::LabelSetImageHelper::SetupDerivedSegmentation(MultiLabelSegmentation* seg,
                                                         const Image* source)
{
  if (seg == nullptr)
    mitkThrow() << "LabelSetImageHelper::SetupDerivedSegmentation: seg must not be nullptr.";
  if (source == nullptr)
    mitkThrow() << "LabelSetImageHelper::SetupDerivedSegmentation: source must not be nullptr.";

  try
  {
    SegSourceImageRelationRule::Connect(seg, source);
    DICOMSegmentationPropertyHelper::InheritPatientFromSource(seg, source);
    DICOMSegmentationPropertyHelper::InheritStudyFromSource(seg, source);
    DICOMSegmentationPropertyHelper::InheritFrameOfReferenceFromSource(seg, source);
  }
  catch (const mitk::Exception& e)
  {
    MITK_WARN << "LabelSetImageHelper::SetupDerivedSegmentation: setup failed: "
              << e.what();
  }
}

mitk::Color mitk::LabelSetImageHelper::SuggestNewLabelColor(const std::vector<mitk::Color>& usedColors)
{
  // Every color already in use, expressed in CIE Lab. Black (the
  // background) is always reserved.
  const double blackRGB[3] = { 0.0, 0.0, 0.0 };
  std::vector<std::array<double, 3>> usedLabColors = { RGBToLab(blackRGB) };

  for (const auto& color : usedColors)
    usedLabColors.push_back(RGBToLab(color));

  // Preserve the historical convention: the first color in an otherwise
  // empty set is palette[0] (the deep red-pink).
  if (1 == usedLabColors.size())
  {
    std::array<double, 3> firstColor{};
    mitk::LookupTable::GetMultiLabelColor(0, firstColor.data());
    return mitk::MakeColor(
      static_cast<float>(firstColor[0]),
      static_cast<float>(firstColor[1]),
      static_cast<float>(firstColor[2]));
  }

  // Maximin selection: pick the candidate whose nearest used-color
  // distance is the largest.
  std::array<double, 3> bestRGB{};
  double                bestMinDistanceSquared = -1.0;

  auto evaluateCandidate = [&](const std::array<double, 3>& candidateRGB)
  {
    const auto candidateLab = RGBToLab(candidateRGB.data());

    double minDistanceSquared = std::numeric_limits<double>::infinity();
    for (const auto& usedLab : usedLabColors)
    {
      const double d2 = DeltaE76Squared(candidateLab, usedLab);
      if (d2 < minDistanceSquared)
        minDistanceSquared = d2;
    }

    if (minDistanceSquared > bestMinDistanceSquared)
    {
      bestMinDistanceSquared = minDistanceSquared;
      bestRGB = candidateRGB;
    }
  };

  // Group A: the curated palette colors (color indices 0..N-1). Always
  // part of the candidate pool. Evaluated first so exact ties favor the
  // curated palette.
  const int paletteColorCount = mitk::LookupTable::GetMultiLabelColorCount();
  std::array<double, 3> palettePick{};
  for (int i = 0; i < paletteColorCount; ++i)
  {
    mitk::LookupTable::GetMultiLabelColor(i, palettePick.data());
    evaluateCandidate(palettePick);
  }

  // Group B: generated colors past the curated palette (color indices
  // paletteColorCount and up). They contribute only once the palette is
  // exhausted, i.e. its best remaining color is no farther from the in-use
  // colors than the palette colors are from one another (see
  // ComputePaletteExhaustedThresholdSquared).
  static const double paletteExhaustedThresholdSquared = ComputePaletteExhaustedThresholdSquared();
  if (bestMinDistanceSquared < paletteExhaustedThresholdSquared)
  {
    constexpr int extraCandidateCount = 1000;
    std::array<double, 3> extraPick{};
    for (int i = 0; i < extraCandidateCount; ++i)
    {
      mitk::LookupTable::GetMultiLabelColor(paletteColorCount + i, extraPick.data());
      evaluateCandidate(extraPick);
    }
  }

  return mitk::MakeColor(
    static_cast<float>(bestRGB[0]),
    static_cast<float>(bestRGB[1]),
    static_cast<float>(bestRGB[2]));
}

mitk::Label::Pointer mitk::LabelSetImageHelper::CreateNewLabel(const MultiLabelSegmentation* labelSetImage, const std::string& namePrefix, bool hideIDIfUnique)
{
  if (nullptr == labelSetImage)
    return nullptr;

  const unsigned int minDigitsCount = 2;
  const std::regex genericLabelNameRegEx(namePrefix + " ([0-9]+)");
  int maxGenericLabelNumber = 0;

  std::vector<mitk::Color> usedColors;
  for (auto & label : labelSetImage->GetLabels())
  {
    auto labelName = label->GetName();
    std::smatch match;

    if (std::regex_match(labelName, match, genericLabelNameRegEx))
      maxGenericLabelNumber = std::max(maxGenericLabelNumber, std::stoi(match[1].str()));

    usedColors.push_back(label->GetColor());
  }

  auto newLabel = mitk::Label::New();
  if (hideIDIfUnique && 0 == maxGenericLabelNumber)
  {
    newLabel->SetName(namePrefix);
  }
  else
  {
    std::ostringstream name;
    name << namePrefix << " " << std::setw(minDigitsCount) << std::setfill('0') << maxGenericLabelNumber + 1;
    newLabel->SetName(name.str().c_str());
  }

  newLabel->SetColor(SuggestNewLabelColor(usedColors));

  return newLabel;
}

mitk::LabelSetImageHelper::GroupIDToLabelValueMapType
mitk::LabelSetImageHelper::SplitLabelValuesByGroup(const MultiLabelSegmentation* labelSetImage, const MultiLabelSegmentation::LabelValueVectorType& labelValues)
{
  if (nullptr == labelSetImage)
    mitkThrow() << "Cannot split label values. Invalid MultiLabelSegmentation pointer passed";

  GroupIDToLabelValueMapType result;

  for (auto value : labelValues)
  {
    auto groupID = labelSetImage->GetGroupIndexOfLabel(value);

    //if groupID does not exist in result this call will also init an empty vector.
    result[groupID].push_back(value);
  }

  return result;
}

mitk::LabelSetImageHelper::LabelClassNameToLabelValueMapType
mitk::LabelSetImageHelper::SplitLabelValuesByClassName(const MultiLabelSegmentation* labelSetImage, MultiLabelSegmentation::GroupIndexType groupID)
{
  if (nullptr == labelSetImage)
    mitkThrow() << "Cannot split label values. Invalid MultiLabelSegmentation pointer passed";

  return SplitLabelValuesByClassName(labelSetImage, groupID, labelSetImage->GetLabelValuesByGroup(groupID));
}

mitk::LabelSetImageHelper::LabelClassNameToLabelValueMapType
mitk::LabelSetImageHelper::SplitLabelValuesByClassName(const MultiLabelSegmentation* labelSetImage, MultiLabelSegmentation::GroupIndexType groupID, const MultiLabelSegmentation::LabelValueVectorType& labelValues)
{
  if (nullptr == labelSetImage)
    mitkThrow() << "Cannot split label values. Invalid MultiLabelSegmentation pointer passed";

  LabelClassNameToLabelValueMapType result;

  for (const auto value : labelValues)
  {
    if (labelSetImage->GetGroupIndexOfLabel(value) == groupID)
    {
      auto className = labelSetImage->GetLabel(value)->GetName();

      //if className does not exist in result this call will also init an empty vector.
      result[className].push_back(value);
    }
  }

  return result;
}

mitk::LabelSetImageHelper::SourceToTargetGroupIDToLabelValueMappingMapType
mitk::LabelSetImageHelper::SplitLabelValueMappingBySourceAndTargetGroup(const MultiLabelSegmentation* sourceSeg, const MultiLabelSegmentation* targetSeg, const LabelValueMappingVector& labelMapping)
{
  SourceToTargetGroupIDToLabelValueMappingMapType result;

  if (nullptr == sourceSeg)
  {
    mitkThrow() << "Invalid call of SplitLabelValueMappingBySourceAndTargetGroup; sourceSeg must not be null.";
  }
  if (nullptr == targetSeg)
  {
    mitkThrow() << "Invalid call of SplitLabelValueMappingBySourceAndTargetGroup; targetSeg must not be null.";
  }

  //split all label mappings by source group id
  using GroupToLabelValueMappingMap = std::map <MultiLabelSegmentation::GroupIndexType, LabelValueMappingVector >;
  GroupToLabelValueMappingMap sourceGroupMappings;
  for (const auto& [sourceLabelValue, targetLabelValue] : labelMapping)
  {
    const auto sourceGroupID = sourceSeg->GetGroupIndexOfLabel(sourceLabelValue);
    const auto targetGroupID = targetSeg->GetGroupIndexOfLabel(targetLabelValue);

    result[sourceGroupID][targetGroupID].emplace_back(sourceLabelValue, targetLabelValue);
  }

  return result;
}


std::string mitk::LabelSetImageHelper::CreateDisplayGroupName(const MultiLabelSegmentation* labelSetImage, MultiLabelSegmentation::GroupIndexType groupID)
{
  const auto groupName = labelSetImage->GetGroupName(groupID);
  if (groupName.empty())
    return "Group "+std::to_string(groupID + 1);

  return groupName;
}

std::string mitk::LabelSetImageHelper::CreateDisplayLabelName(const MultiLabelSegmentation* labelSetImage, const Label* label)
{
  auto labelName = label->GetName();

  if (labelName.empty())
    labelName = "Unnamed";

  if (nullptr != labelSetImage &&
    labelSetImage->GetLabelValuesByName(labelSetImage->GetGroupIndexOfLabel(label->GetValue()), label->GetName()).size() > 1)
  {
    if (!label->GetTrackingID().empty())
      labelName += " [ID: " + label->GetTrackingID() + ']';
    else
      labelName += " [" + std::to_string(label->GetValue()) + ']';
  }

  return labelName;
}

std::string mitk::LabelSetImageHelper::CreateHTMLLabelName(const mitk::Label* label, const mitk::MultiLabelSegmentation* segmentation)
{
  std::stringstream stream;
  auto color = label->GetColor();
  stream << "<span style='color: #" << std::hex << std::setfill('0')
    << std::setw(2) << static_cast<int>(color.GetRed() * 255)
    << std::setw(2) << static_cast<int>(color.GetGreen() * 255)
    << std::setw(2) << static_cast<int>(color.GetBlue() * 255)
    << "; font-size: 20px '>&#x25A0;</span>" << std::dec;

  stream << "<font class=\"normal\"> " << CreateDisplayLabelName(segmentation, label);
  stream << "</font>";
  return stream.str();
}

std::string mitk::LabelSetImageHelper::CreateHTMLLabelDetails(const mitk::Label* label, const mitk::MultiLabelSegmentation* segmentation)
{
  std::stringstream stream;
  stream << "<font class=\"normal\"> ";

  stream << "<b>Pixel value:</b> " << label->GetValue();
  if (nullptr != segmentation && segmentation->GetNumberOfGroups() > 1 && segmentation->ExistLabel(label->GetValue()))
  {
    stream << "<br/><b>Group:</b> " << CreateDisplayGroupName(segmentation, segmentation->GetGroupIndexOfLabel(label->GetValue()));
  }
  if (!label->GetTrackingID().empty())
  {
    stream <<"<br/><b>Tracking ID:</b> "<< label->GetTrackingID();
  }
  if (label->GetAnatomicRegionCount() > 0)
  {
    auto code = label->GetAnatomicRegion(0);
    stream <<"<br/><b>Anatomic region:</b> " << code.GetMeaning();
  }
  if (label->GetPrimaryAnatomicStructureCount() > 0)
  {
    auto code = label->GetPrimaryAnatomicStructure(0);
    stream << "<br/><b>Primary anatomic structure:</b> " << code.GetMeaning();
    if (code.GetModifierCount() > 0)
    {
      stream << " (" <<code.GetModifier(0).GetMeaning()<<")";
    }
  }
  if (auto code = label->GetSegmentedPropertyCategory(); code.has_value())
  {
    stream <<"<br/><b>Type category:</b> " << code->GetMeaning();
  }
  if (auto code = label->GetSegmentedPropertyType(); code.has_value())
  {
    stream << "<br/><b>Type:</b> " << code->GetMeaning();
    if (code->GetModifierCount() > 0)
    {
      stream << " (" << code->GetModifier(0).GetMeaning() << ")";
    }
  }
  if (!label->GetDescription().empty())
  {
    stream <<"<br/><b>Description:</b> " << label->GetDescription();
  }

  stream << "</font>";
  return stream.str();
}
