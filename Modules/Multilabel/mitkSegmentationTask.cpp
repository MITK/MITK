/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkSegmentationTask.h"

#include <mitkIOUtil.h>
#include <mitkProperties.h>

mitk::SegmentationTask::Subtask::Subtask()
  : m_Defaults(nullptr)
{
}

mitk::SegmentationTask::Subtask::~Subtask()
{
}

void mitk::SegmentationTask::Subtask::SetDefaults(const Subtask* defaults)
{
  m_Defaults = defaults;
}

mitk::SegmentationTask::SegmentationTask()
{
  // A base data cannot be serialized if empty. To be not considered empty its
  // geometry must consist of at least one time step. However, a segmentation
  // task would then appear as invisible spacial object in a scene. This can
  // be prevented by excluding it from the scene's bounding box calculations.
  this->GetTimeGeometry()->Expand(1);
  this->SetProperty("includeInBoundingBox", BoolProperty::New(false));
}

mitk::SegmentationTask::SegmentationTask(const Self& other)
  : BaseData(other)
{
}

mitk::SegmentationTask::~SegmentationTask()
{
}

size_t mitk::SegmentationTask::GetNumberOfSubtasks() const
{
  return m_Subtasks.size();
}

size_t mitk::SegmentationTask::AddSubtask(const Subtask& subtask)
{
  m_Subtasks.push_back(subtask);
  m_Subtasks.back().SetDefaults(&m_Defaults);
  return m_Subtasks.size() - 1;
}

const mitk::SegmentationTask::Subtask* mitk::SegmentationTask::GetSubtask(size_t index) const
{
  return &m_Subtasks.at(index);
}

mitk::SegmentationTask::Subtask* mitk::SegmentationTask::GetSubtask(size_t index)
{
  return &m_Subtasks.at(index);
}

const mitk::SegmentationTask::Subtask& mitk::SegmentationTask::GetDefaults() const
{
  return m_Defaults;
}

void mitk::SegmentationTask::SetDefaults(const Subtask& defaults)
{
  m_Defaults = defaults;

  for (auto& subtask : m_Subtasks)
    subtask.SetDefaults(&m_Defaults);
}

bool mitk::SegmentationTask::IsDone() const
{
  for (size_t i = 0; i < m_Subtasks.size(); ++i)
  {
    if (!this->IsDone(i))
      return false;
  }

  return true;
}

bool mitk::SegmentationTask::IsDone(size_t index) const
{
  return std::filesystem::exists(this->GetAbsolutePath(m_Subtasks.at(index).GetResult()));
}

std::filesystem::path mitk::SegmentationTask::GetInputLocation() const
{
  std::string result;
  this->GetPropertyList()->GetStringProperty("MITK.IO.reader.inputlocation", result);

  return !result.empty()
    ? std::filesystem::path(result).lexically_normal()
    : result;
}

std::filesystem::path mitk::SegmentationTask::GetBasePath() const
{
  return this->GetInputLocation().remove_filename();
}

std::filesystem::path mitk::SegmentationTask::GetAbsolutePath(const std::filesystem::path& path) const
{
  if (path.empty())
    return path;

  auto normalizedPath = path.lexically_normal();

  return !normalizedPath.is_absolute()
    ? this->GetBasePath() / normalizedPath
    : normalizedPath;
}

void mitk::SegmentationTask::SaveSubtask(size_t index, const BaseData* segmentation)
{
  if (segmentation == nullptr)
    return;

  auto path = this->GetAbsolutePath(this->GetResult(index));
  IOUtil::Save(segmentation, path.string());
}

std::vector<mitk::SegmentationTask::Subtask>::const_iterator mitk::SegmentationTask::begin() const
{
  return m_Subtasks.begin();
}

std::vector<mitk::SegmentationTask::Subtask>::const_iterator mitk::SegmentationTask::end() const
{
  return m_Subtasks.end();
}

std::vector<mitk::SegmentationTask::Subtask>::iterator mitk::SegmentationTask::begin()
{
  return m_Subtasks.begin();
}

std::vector<mitk::SegmentationTask::Subtask>::iterator mitk::SegmentationTask::end()
{
  return m_Subtasks.end();
}

void mitk::SegmentationTask::SetRequestedRegionToLargestPossibleRegion()
{
}

bool mitk::SegmentationTask::RequestedRegionIsOutsideOfTheBufferedRegion()
{
  return false;
}

bool mitk::SegmentationTask::VerifyRequestedRegion()
{
  return true;
}

void mitk::SegmentationTask::SetRequestedRegion(const itk::DataObject*)
{
}
