/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkDICOMSplitReason.h"

#include <nlohmann/json.hpp>

void mitk::DICOMSplitReason::AddReason(ReasonType type, std::string_view detail)
{
  m_ReasonMap.insert(std::make_pair(type, detail));
}

void mitk::DICOMSplitReason::RemoveReason(ReasonType type)
{
  m_ReasonMap.erase(type);
}

bool mitk::DICOMSplitReason::ReasonExists() const
{
  return !m_ReasonMap.empty();
}

bool mitk::DICOMSplitReason::ReasonExists(ReasonType type) const
{
  return m_ReasonMap.cend() != m_ReasonMap.find(type);
}

std::string mitk::DICOMSplitReason::GetReasonDetails(ReasonType type) const
{
  auto finding = m_ReasonMap.find(type);
  if (m_ReasonMap.cend() == finding)
    mitkThrow() << "Cannot get details for inexistent type.";

  return finding->second;
};

mitk::DICOMSplitReason::Pointer mitk::DICOMSplitReason::ExtendReason(const Self* otherReason) const
{
  if (nullptr == otherReason)
    mitkThrow() << "Cannot extend reason. Pass other reason is in valid.";

  Pointer result = this->Clone();

  result->m_ReasonMap.insert(otherReason->m_ReasonMap.cbegin(), otherReason->m_ReasonMap.cend());

  return result;
}

mitk::DICOMSplitReason::DICOMSplitReason(): itk::LightObject()
{
}

mitk::DICOMSplitReason::~DICOMSplitReason()
{
}

mitk::DICOMSplitReason::DICOMSplitReason(const DICOMSplitReason& other)
{
  m_ReasonMap = other.m_ReasonMap;
}

std::string mitk::DICOMSplitReason::TypeToString(const DICOMSplitReason::ReasonType& reasonType)
{
  switch (reasonType)
  {
  case DICOMSplitReason::ReasonType::GantryTiltDifference:
    return "gantry_tilt_difference";
  case DICOMSplitReason::ReasonType::ImagePostionMissing:
    return "gantry_tilt_difference";
  case DICOMSplitReason::ReasonType::OverlappingSlices:
    return "overlapping_slices";
  case DICOMSplitReason::ReasonType::SliceDistanceInconsistency:
    return "slice_distance_inconsistency";
  case DICOMSplitReason::ReasonType::ValueSortDistance:
    return "value_sort_distance";
  case DICOMSplitReason::ReasonType::ValueSplitDifference:
    return "value_split_difference";
  }
  return "unknown";
}

std::string mitk::DICOMSplitReason::SerializeToJSON(const DICOMSplitReason* reason)
{
  if (nullptr == reason)
    mitkThrow() << "Cannot extend reason. Pass other reason is in valid.";

  auto data = nlohmann::json::array();

  for (const auto& [type, detail] : reason->m_ReasonMap)
  {
    auto details = nlohmann::json::array();

    details.push_back(TypeToString(type));
    details.push_back(detail);
    data.push_back(details);
  }

  return data.dump();
}
