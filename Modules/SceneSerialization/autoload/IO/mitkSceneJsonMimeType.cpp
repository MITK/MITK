/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkSceneJsonMimeType.h"

#include <mitkFileSystem.h>
#include <mitkIOMimeTypes.h>
#include <mitkLog.h>

#include <fstream>
#include <string>

namespace
{
  // Sniff the first few KB for the root "type": "org.mitk.scene" marker.
  bool FileContainsSceneMarker(const std::string &path)
  {
    std::ifstream in(path, std::ios::binary);
    if (!in.good())
      return false;

    constexpr std::size_t kSniffBytes = 4096;
    std::string buffer(kSniffBytes, '\0');
    in.read(buffer.data(), kSniffBytes);
    buffer.resize(static_cast<std::size_t>(in.gcount()));

    return buffer.find("\"org.mitk.scene\"") != std::string::npos;
  }
}

mitk::SceneJsonMimeType::SceneJsonMimeType()
  : CustomMimeType(Name())
{
  this->AddExtension("mitkscene.json");
  this->SetCategory("MITK Scenes");
  this->SetComment("MITK JSON scene file");
}

bool mitk::SceneJsonMimeType::AppliesTo(const std::string &path) const
{
  if (!CustomMimeType::AppliesTo(path))
    return false;

  if (!fs::exists(path))
    return true; // extension-only check when the file is not yet present

  return FileContainsSceneMarker(path);
}

mitk::SceneJsonMimeType *mitk::SceneJsonMimeType::Clone() const
{
  return new SceneJsonMimeType(*this);
}

std::string mitk::SceneJsonMimeType::Name()
{
  return IOMimeTypes::DEFAULT_BASE_NAME() + ".scene.json";
}
