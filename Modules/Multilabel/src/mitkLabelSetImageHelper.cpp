/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkLabelSetImageHelper.h>

#include <mitkDataStorage.h>
#include <mitkLabelSetImage.h>
#include <mitkExceptionMacro.h>
#include <mitkProperties.h>

#include <vtkMath.h>

#include <array>
#include <cmath>
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

  // Standard 6-sector HSV->RGB. Kept local so we don't pull in
  // MitkDataTypesExt for fifteen lines of math.
  std::array<double, 3> HSVToRGB(double h, double s, double v)
  {
    h = h - std::floor(h);

    const double sector      = h * 6.0;
    const int    sectorIndex = static_cast<int>(std::floor(sector)) % 6;
    const double fractional  = sector - std::floor(sector);

    const double p = v * (1.0 - s);
    const double q = v * (1.0 - s * fractional);
    const double t = v * (1.0 - s * (1.0 - fractional));

    switch (sectorIndex)
    {
      case 0:  return { v, t, p };
      case 1:  return { q, v, p };
      case 2:  return { p, v, t };
      case 3:  return { p, q, v };
      case 4:  return { t, p, v };
      default: return { v, p, q };
    }
  }

  // Number of algorithmically generated extra candidates evaluated when
  // the palette is exhausted. Sized to comfortably cover any realistic
  // segmentation: with all 25 palette colors used and N extras, the
  // pool stays distinct up to ~(25 + N) labels before the maximin can
  // no longer find a non-duplicate. The golden-angle hue sequence
  // produces a distinct hue at every index, so this can be cranked up
  // freely; the only cost is one Lab distance computation per extra
  // per used color, which is sub-millisecond even at N = 1000.
  constexpr int EXTRA_CANDIDATE_COUNT = 1000;

  // Golden-ratio conjugate: hue step that yields a low-discrepancy
  // sequence on the unit circle. Any prefix stays maximally even.
  constexpr double GOLDEN_HUE_STEP = 0.6180339887498949;

  // i-th extra candidate (0-indexed), cycling through three saturation/
  // value tiers and advancing hue by the golden angle each step.
  std::array<double, 3> GenerateExtraCandidate(int i)
  {
    struct Tier { double saturation; double value; };
    constexpr std::array<Tier, 3> tiers = { {
      { 0.85, 0.95 },
      { 0.55, 0.95 },
      { 0.85, 0.60 }
    } };

    const Tier& tier = tiers[i % tiers.size()];
    const double hue = std::fmod(i * GOLDEN_HUE_STEP, 1.0);
    return HSVToRGB(hue, tier.saturation, tier.value);
  }

  mitk::Color FromLookupTableColor(const double* lookupTableColor)
  {
    mitk::Color color;
    color.Set(
      static_cast<float>(lookupTableColor[0]),
      static_cast<float>(lookupTableColor[1]),
      static_cast<float>(lookupTableColor[2]));
    return color;
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

  return newSegmentationNode;
}

mitk::Label::Pointer mitk::LabelSetImageHelper::CreateNewLabel(const MultiLabelSegmentation* labelSetImage, const std::string& namePrefix, bool hideIDIfUnique)
{
  if (nullptr == labelSetImage)
    return nullptr;

  const unsigned int minDigitsCount = 2;
  const std::regex genericLabelNameRegEx(namePrefix + " ([0-9]+)");
  int maxGenericLabelNumber = 0;

  // Every color already in use, expressed in CIE Lab. Black (the
  // background) is always reserved.
  const double blackRGB[3] = { 0.0, 0.0, 0.0 };
  std::vector<std::array<double, 3>> usedLabColors = { RGBToLab(blackRGB) };

  for (auto & label : labelSetImage->GetLabels())
  {
    auto labelName = label->GetName();
    std::smatch match;

    if (std::regex_match(labelName, match, genericLabelNameRegEx))
      maxGenericLabelNumber = std::max(maxGenericLabelNumber, std::stoi(match[1].str()));

    usedLabColors.push_back(RGBToLab(label->GetColor()));
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

  auto lookupTable = mitk::LookupTable::New();
  lookupTable->SetType(mitk::LookupTable::LookupTableType::MULTILABEL);

  // Preserve the historical convention: the first label in an empty
  // segmentation is palette[0] (lookup-table slot 1, the deep red-pink).
  if (1 == usedLabColors.size())
  {
    std::array<double, 3> firstColor{};
    lookupTable->GetColor(1, firstColor.data());
    newLabel->SetColor(FromLookupTableColor(firstColor.data()));
    return newLabel;
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

  // Group A: the curated palette colors (lookup-table slots 1..N).
  // Always part of the candidate pool. Evaluated first so exact ties
  // favor the curated palette.
  const int paletteColorCount = mitk::LookupTable::GetMultiLabelColorCount();
  std::array<double, 3> palettePick{};
  for (int i = 1; i <= paletteColorCount; ++i)
  {
    lookupTable->GetColor(i, palettePick.data());
    evaluateCandidate(palettePick);
  }

  // Group B: algorithmically generated extras. Only contributes when the
  // palette has no candidate left at a meaningful Lab distance from the
  // colors in use. The threshold is set just below the smallest pairwise
  // ΔE76 within the palette (palette[0] #BE0032 vs palette[16] #BF5D36,
  // ΔE76 ≈ 32.4, squared ≈ 1051.7), so every palette color can still win
  // on its own merits before we extend with extras.
  constexpr double PALETTE_EXHAUSTED_THRESHOLD_SQUARED = 1040.0;
  if (bestMinDistanceSquared < PALETTE_EXHAUSTED_THRESHOLD_SQUARED)
  {
    for (int i = 0; i < EXTRA_CANDIDATE_COUNT; ++i)
    {
      evaluateCandidate(GenerateExtraCandidate(i));
    }
  }

  newLabel->SetColor(FromLookupTableColor(bestRGB.data()));

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
