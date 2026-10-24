/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkPlanarFigureIOMimeTypes.h"

#include <mitkFileSystem.h>
#include <mitkIOMimeTypes.h>

#include <nlohmann/json.hpp>

#include <fstream>

mitk::PlanarFigureIOMimeTypes::PlanarFigureMimeType::PlanarFigureMimeType()
  : CustomMimeType(PLANARFIGURE_MIMETYPE_NAME())
{
  this->AddExtension("pf.json");
  this->AddExtension("json");
  this->SetCategory("MITK Planar Figure");
  this->SetComment("MITK Planar Figure");
}

bool mitk::PlanarFigureIOMimeTypes::PlanarFigureMimeType::AppliesTo(const std::string &path) const
{
  const auto result = CustomMimeType::AppliesTo(path);

  // The dedicated extension needs no look into the file, which may also be
  // about to be overwritten.
  if (!result || !fs::exists(path) || ".pf.json" == this->GetExtension(path))
    return result;

  if (fs::is_directory(path))
    return false;

  std::ifstream file(path);

  if (!file.is_open())
    return false;

  const auto json = nlohmann::json::parse(file, nullptr, false);

  // The version is left to the reader, which reports files of newer versions.
  return !json.is_discarded() && json.is_object() && FileFormat == json.value("FileFormat", "");
}

mitk::PlanarFigureIOMimeTypes::PlanarFigureMimeType *mitk::PlanarFigureIOMimeTypes::PlanarFigureMimeType::Clone() const
{
  return new PlanarFigureMimeType(*this);
}

mitk::PlanarFigureIOMimeTypes::PlanarFigureMimeType mitk::PlanarFigureIOMimeTypes::PLANARFIGURE_MIMETYPE()
{
  return PlanarFigureMimeType();
}

std::string mitk::PlanarFigureIOMimeTypes::PLANARFIGURE_MIMETYPE_NAME()
{
  return IOMimeTypes::DEFAULT_BASE_NAME() + ".planarfigure";
}

mitk::CustomMimeType mitk::PlanarFigureIOMimeTypes::LEGACY_PLANARFIGURE_MIMETYPE()
{
  CustomMimeType mimeType(LEGACY_PLANARFIGURE_MIMETYPE_NAME());
  mimeType.AddExtension("pf");
  mimeType.SetCategory("MITK Planar Figure");
  mimeType.SetComment("MITK Planar Figure (XML)");
  return mimeType;
}

std::string mitk::PlanarFigureIOMimeTypes::LEGACY_PLANARFIGURE_MIMETYPE_NAME()
{
  return IOMimeTypes::DEFAULT_BASE_NAME() + ".planarfigure.legacy";
}
