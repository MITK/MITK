/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkSegmentationTaskIO.h"
#include "mitkMultilabelIOMimeTypes.h"

#include <mitkSegmentationTask.h>

#include <nlohmann/json.hpp>

#include <filesystem>
#include <fstream>

mitk::SegmentationTaskIO::SegmentationTaskIO()
  : AbstractFileIO(SegmentationTask::GetStaticNameOfClass(), MitkMultilabelIOMimeTypes::SEGMENTATIONTASK_MIMETYPE(), "MITK Segmentation Task")
{
  this->RegisterService();
}

std::vector<mitk::BaseData::Pointer> mitk::SegmentationTaskIO::DoRead()
{
  auto* stream = this->GetInputStream();
  std::ifstream fileStream;

  if (nullptr == stream)
  {
    auto filename = this->GetInputLocation();

    if (filename.empty() || !std::filesystem::exists(filename))
      mitkThrow() << "Invalid or nonexistent filename: \"" << filename << "\"!";

    fileStream.open(filename);

    if (!fileStream.is_open())
      mitkThrow() << "Could not open file \"" << filename << "\" for reading!";

    stream = &fileStream;
  }

  auto json = nlohmann::json::parse(*stream, nullptr, false);

  if (json.is_discarded() || !json.is_object())
    mitkThrow() << "Could not parse JSON!";

  if ("MITK Segmentation Task" != json.value("FileFormat", ""))
    mitkThrow() << "Unknown file format (expected \"MITK Segmentation Task\")!";

  if (1 != json.value<int>("Version", 0))
    mitkThrow() << "Unknown file format version (expected \"1\")!";

  auto segmentationTask = SegmentationTask::New();

  // TODO

  std::vector<BaseData::Pointer> result;
  result.push_back(segmentationTask.GetPointer());

  return result;
}

void mitk::SegmentationTaskIO::Write()
{
  auto* stream = this->GetOutputStream();
  std::ofstream fileStream;

  if (nullptr == stream)
  {
    auto filename = this->GetOutputLocation();

    if (filename.empty())
      mitkThrow() << "Neither an output stream nor an output filename was specified!";

    fileStream.open(filename);

    if (!fileStream.is_open())
      mitkThrow() << "Could not open file \"" << filename << "\" for writing!";

    stream = &fileStream;
  }

  if (!stream->good())
    mitkThrow() << "Stream for writing is not good!";

  nlohmann::json json = {
    { "FileFormat", "MITK Segmentation Task" },
    { "Version", 1 }
  };

  // TODO

  *stream << std::setw(2) << json << std::endl;
}

mitk::SegmentationTaskIO* mitk::SegmentationTaskIO::IOClone() const
{
  return new SegmentationTaskIO(*this);
}
