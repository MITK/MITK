/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkIOVolumeSplitReason.h>

#include <nlohmann/json.hpp>

void mitk::IOVolumeSplitReason::AddReason(ReasonType type, const std::string& detail)
{
  m_ReasonMap[type] = detail;
}

void mitk::IOVolumeSplitReason::RemoveReason(ReasonType type)
{
  m_ReasonMap.erase(type);
}

bool mitk::IOVolumeSplitReason::HasReasons() const
{
  return !m_ReasonMap.empty();
}

bool mitk::IOVolumeSplitReason::HasReason(ReasonType type) const
{
  return m_ReasonMap.cend() != m_ReasonMap.find(type);
}

std::string mitk::IOVolumeSplitReason::GetReasonDetails(ReasonType type) const
{
  auto finding = m_ReasonMap.find(type);
  if (m_ReasonMap.cend() == finding)
    mitkThrow() << "Cannot get details for inexistent type.";

  return finding->second;
};

mitk::IOVolumeSplitReason::Pointer mitk::IOVolumeSplitReason::ExtendReason(const Self* otherReason) const
{
  if (nullptr == otherReason)
    mitkThrow() << "Cannot extend reason. Passed other reason is in valid.";

  Pointer result = this->Clone();

  result->m_ReasonMap.insert(otherReason->m_ReasonMap.cbegin(), otherReason->m_ReasonMap.cend());

  return result;
}

mitk::IOVolumeSplitReason::IOVolumeSplitReason(): itk::LightObject()
{
}

mitk::IOVolumeSplitReason::~IOVolumeSplitReason()
{
}

mitk::IOVolumeSplitReason::IOVolumeSplitReason(const IOVolumeSplitReason& other) : m_ReasonMap(other.m_ReasonMap)
{
}

std::string mitk::IOVolumeSplitReason::TypeToString(ReasonType reasonType)
{
  switch (reasonType)
  {
  case ReasonType::GantryTiltDifference:
    return "gantry_tilt_difference";
  case ReasonType::ImagePostionMissing:
    return "image_position_missing";
  case ReasonType::OverlappingSlices:
    return "overlapping_slices";
  case ReasonType::SliceDistanceInconsistency:
    return "slice_distance_inconsistency";
  case ReasonType::ValueSortDistance:
    return "value_sort_distance";
  case ReasonType::ValueSplitDifference:
    return "value_split_difference";
  case ReasonType::MissingSlices:
    return "missing_slices";
  default: return "unknown";
  }
}

mitk::IOVolumeSplitReason::ReasonType mitk::IOVolumeSplitReason::StringToType(const std::string& reasonStr)
{
  if (reasonStr == "gantry_tilt_difference")
    return ReasonType::GantryTiltDifference;
  else if (reasonStr == "image_position_missing")
    return ReasonType::ImagePostionMissing;
  else if (reasonStr == "overlapping_slices")
    return ReasonType::OverlappingSlices;
  else if (reasonStr == "slice_distance_inconsistency")
    return ReasonType::SliceDistanceInconsistency;
  else if (reasonStr == "value_sort_distance")
    return ReasonType::ValueSortDistance;
  else if (reasonStr == "value_split_difference")
    return ReasonType::ValueSplitDifference;
  else if (reasonStr == "missing_slices")
    return ReasonType::MissingSlices;

  return ReasonType::Unknown;
}

std::string mitk::IOVolumeSplitReason::ToJSON(const IOVolumeSplitReason* reason)
{
  if (nullptr == reason)
    mitkThrow() << "Cannot extend reason. Passed other reason is in valid.";

  auto data = nlohmann::json::array();

  for (const auto& [type, detail] : reason->m_ReasonMap)
  {
    auto details = nlohmann::json::array();

    details.push_back(TypeToString(type));
    if (!detail.empty())
    {
      details.push_back(detail);
    }
    data.push_back(details);
  }

  return data.dump();
}

mitk::IOVolumeSplitReason::Pointer mitk::IOVolumeSplitReason::FromJSON(const std::string& reasonStr)
{
  if (reasonStr.empty())
    return nullptr;

  auto reason = IOVolumeSplitReason::New();

  auto data = nlohmann::json::parse(reasonStr);

  for (const auto& jItem : data)
  {
    if (!jItem.empty())
    {
      ReasonType reasonType = StringToType(jItem.at(0).get<std::string>());

      std::string detail;
      if (jItem.size() > 1)
      {
        detail = jItem.at(1).get<std::string>();
      }
      reason->AddReason(reasonType, detail);
    }
  }

  return reason;
}
