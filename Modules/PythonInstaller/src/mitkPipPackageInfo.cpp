/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkPipPackageInfo.h>

#include <nlohmann/json.hpp>

#include <QFile>

#include <fstream>
#include <stdexcept>

mitk::PipInstallGroup::PipInstallGroup(std::initializer_list<std::string> requirements)
  : requirements(requirements)
{
}

// --- PipInstallGroup JSON ---

void mitk::from_json(const nlohmann::ordered_json& j, PipInstallGroup& g)
{
  j.at("requirements").get_to(g.requirements);
  g.indexUrl = j.value("indexUrl", std::string());
  g.extraPipArgs = j.value("extraPipArgs", std::vector<std::string>());
}

void mitk::to_json(nlohmann::ordered_json& j, const PipInstallGroup& g)
{
  j["requirements"] = g.requirements;
  j["indexUrl"] = g.indexUrl;
  j["extraPipArgs"] = g.extraPipArgs;
}

// --- HuggingFaceDownload JSON ---

void mitk::from_json(const nlohmann::ordered_json& j, HuggingFaceDownload& d)
{
  j.at("repoId").get_to(d.repoId);
  d.allowPatterns = j.value("allowPatterns", std::vector<std::string>());
  d.displayName = j.value("displayName", std::string());
  d.optional = j.value("optional", false);
}

void mitk::to_json(nlohmann::ordered_json& j, const HuggingFaceDownload& d)
{
  j["repoId"] = d.repoId;
  j["allowPatterns"] = d.allowPatterns;
  j["displayName"] = d.displayName;
  j["optional"] = d.optional;
}

// --- PipInstallSpec JSON ---

static constexpr auto FILE_FORMAT = "MITK PipInstallSpec";
static constexpr int FILE_VERSION = 1;

void mitk::from_json(const nlohmann::ordered_json& j, PipInstallSpec& s)
{
  if (j.contains("FileFormat") && j["FileFormat"].get<std::string>() != FILE_FORMAT)
    throw std::runtime_error("Unexpected FileFormat: " + j["FileFormat"].get<std::string>());

  if (j.contains("Version") && j["Version"].get<int>() != FILE_VERSION)
    throw std::runtime_error("Unsupported PipInstallSpec version: " + std::to_string(j["Version"].get<int>()));

  s.name = j.value("name", std::string());
  s.venvName = j.value("venvName", std::string());
  s.groups = j.value("groups", std::vector<PipInstallGroup>());
  s.upgradePipFirst = j.value("upgradePipFirst", true);
  s.huggingFaceDownloads = j.value("huggingFaceDownloads", std::vector<HuggingFaceDownload>());
}

void mitk::to_json(nlohmann::ordered_json& j, const PipInstallSpec& s)
{
  j["FileFormat"] = FILE_FORMAT;
  j["Version"] = FILE_VERSION;
  j["name"] = s.name;
  j["venvName"] = s.venvName;
  j["upgradePipFirst"] = s.upgradePipFirst;
  j["groups"] = s.groups;
  j["huggingFaceDownloads"] = s.huggingFaceDownloads;
}

// --- Static factory methods ---

mitk::PipInstallSpec mitk::PipInstallSpec::FromFile(const std::string& path)
{
  std::ifstream file(path);

  if (!file.is_open())
    throw std::runtime_error("Could not open file: " + path);

  auto j = nlohmann::ordered_json::parse(file);
  PipInstallSpec spec;
  from_json(j, spec);
  return spec;
}

mitk::PipInstallSpec mitk::PipInstallSpec::FromResource(const std::string& resourcePath)
{
  QFile file(QString::fromStdString(resourcePath));

  if (!file.open(QIODevice::ReadOnly))
    throw std::runtime_error("Could not open resource: " + resourcePath);

  auto data = file.readAll();
  auto j = nlohmann::ordered_json::parse(data.constData(), data.constData() + data.size());
  PipInstallSpec spec;
  from_json(j, spec);
  return spec;
}

void mitk::PipInstallSpec::SaveToFile(const std::string& path) const
{
  nlohmann::ordered_json j;
  to_json(j, *this);

  std::ofstream file(path);

  if (!file.is_open())
    throw std::runtime_error("Could not open file for writing: " + path);

  file << j.dump(2);
}
