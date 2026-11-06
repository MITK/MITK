/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <QmitkMxNPresets.h>

#include <mitkExceptionMacro.h>

#include <algorithm>
#include <regex>

#include <usGetModuleContext.h>
#include <usModuleContext.h>
#include <usModuleResource.h>
#include <usModuleResourceStream.h>

namespace
{
  /** Replaces the editor segment of every window id below 'node'. An id without
   *  the "__" separator is left for the loader to reject. */
  void RequalifyWindowIds(nlohmann::json& node, const std::string& widgetName)
  {
    if (const auto id = node.find("id"); node.end() != id && id->is_string())
    {
      const auto separator = id->get_ref<const std::string&>().find("__");
      if (std::string::npos != separator)
      {
        *id = widgetName + id->get_ref<const std::string&>().substr(separator);
      }
    }
    if (const auto children = node.find("children"); node.end() != children && children->is_array())
    {
      for (auto& child : *children)
      {
        RequalifyWindowIds(child, widgetName);
      }
    }
  }

  /** The file name without its ordering prefix and extension. */
  QString PresetId(const us::ModuleResource& resource)
  {
    static const std::regex orderedName("mxnLayout_[0-9]{2}_(.+)");
    const auto baseName = resource.GetCompleteBaseName();
    std::smatch match;
    if (!std::regex_match(baseName, match, orderedName))
    {
      mitkThrow() << "MxN preset '" << resource.GetName() << "' is not named mxnLayout_<two digits>_<id>.json.";
    }
    return QString::fromStdString(match[1].str());
  }

  std::vector<QmitkMxNPresets::Preset> ReadPresets()
  {
    auto resources = us::GetModuleContext()->GetModule()->FindResources("/", "mxnLayout_*.json", false);
    // FindResources promises no order; the two-digit prefixes of the file names
    // define the menu order, and ModuleResource orders by resource path.
    std::sort(resources.begin(), resources.end());

    std::vector<QmitkMxNPresets::Preset> presets;
    presets.reserve(resources.size());
    for (const auto& resource : resources)
    {
      us::ModuleResourceStream stream(resource);
      auto document = nlohmann::json::parse(stream);
      presets.push_back({ PresetId(resource), QString::fromStdString(document["name"].get<std::string>()),
                          std::move(document) });
    }
    return presets;
  }
}

const std::vector<QmitkMxNPresets::Preset>& QmitkMxNPresets::All()
{
  static const auto presets = ReadPresets();
  return presets;
}

nlohmann::json QmitkMxNPresets::DocumentFor(const Preset& preset, const QString& widgetName)
{
  auto document = preset.document;
  if (auto root = document.find("root"); document.end() != root)
  {
    RequalifyWindowIds(*root, widgetName.toStdString());
  }
  return document;
}
