/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitknnInteractiveModel.h>

#include <mitkException.h>
#include <mitkLog.h>
#include <mitkPythonContext.h>

#include <sstream>

namespace
{
  // Split on '\n' into exactly the requested number of rows. list_models() entries
  // never contain newlines (ids match nnInteractive_*, display names are short
  // manifest labels), so a newline-joined string round-trips one entry per row.
  std::vector<std::string> SplitRows(const std::string& joined, std::size_t count)
  {
    std::vector<std::string> rows;
    rows.reserve(count);

    if (count == 0)
      return rows;

    std::istringstream stream(joined);
    std::string row;
    while (rows.size() < count && std::getline(stream, row))
      rows.push_back(row);

    rows.resize(count); // Pad with empty rows if the readback was short.
    return rows;
  }
}

std::vector<mitk::nnInteractive::ModelInfo> mitk::nnInteractive::ListModels(PythonContext& context)
{
  std::vector<ModelInfo> models;

  // Marshal list_models() (a list of dicts) into scalar variables: PythonContext
  // exposes only scalar getters, so the rows are joined with '\n' and split back
  // in C++. A client-only install has no model_management module; any failure
  // (ImportError, offline manifest refresh) leaves the count at 0.
  const std::string pyCommands =
    "nni_models_count = 0\n"
    "nni_models_ids = ''\n"
    "nni_models_names = ''\n"
    "nni_models_defaults = ''\n"
    "nni_models_downloaded = ''\n"
    "try:\n"
    "    from nnInteractive.model_management import list_models\n"
    "    _nni_models = list_models()\n"
    "    nni_models_count = len(_nni_models)\n"
    "    nni_models_ids = '\\n'.join(str(_m.get('id', '')) for _m in _nni_models)\n"
    "    nni_models_names = '\\n'.join(str(_m.get('display_name', '') or _m.get('id', '')) for _m in _nni_models)\n"
    "    nni_models_defaults = '\\n'.join('1' if _m.get('default') else '0' for _m in _nni_models)\n"
    "    nni_models_downloaded = '\\n'.join('1' if _m.get('downloaded') else '0' for _m in _nni_models)\n"
    "except Exception:\n"
    "    nni_models_count = 0\n";

  try
  {
    context.Execute(pyCommands);

    const auto count = context.GetVariableAsInt("nni_models_count").value_or(0);

    if (count > 0)
    {
      const auto n = static_cast<std::size_t>(count);
      const auto ids = SplitRows(context.GetVariableAsString("nni_models_ids").value_or(""), n);
      const auto names = SplitRows(context.GetVariableAsString("nni_models_names").value_or(""), n);
      const auto defaults = SplitRows(context.GetVariableAsString("nni_models_defaults").value_or(""), n);
      const auto downloaded = SplitRows(context.GetVariableAsString("nni_models_downloaded").value_or(""), n);

      for (std::size_t i = 0; i < n; ++i)
      {
        if (ids[i].empty())
          continue; // Skip malformed entries with no id.

        ModelInfo info;
        info.Id = ids[i];
        info.DisplayName = names[i].empty() ? ids[i] : names[i];
        info.IsDefault = defaults[i] == "1";
        info.Downloaded = downloaded[i] == "1";
        models.push_back(std::move(info));
      }
    }

    context.Execute(
      "del nni_models_count, nni_models_ids, nni_models_names, "
      "nni_models_defaults, nni_models_downloaded\n");
  }
  catch (const Exception& e)
  {
    MITK_WARN << "nnInteractive: could not list models: " << e.GetDescription();
  }

  return models;
}

std::vector<mitk::nnInteractive::ModelInfo> mitk::nnInteractive::ListModels()
{
  try
  {
    PythonContext context("nnInteractive");
    context.Activate();

    return ListModels(context);
  }
  catch (const Exception& e)
  {
    MITK_ERROR << e.GetDescription();
    return {};
  }
  catch (...)
  {
    // The PythonContext constructor can throw a non-mitk exception when the
    // embedded interpreter is initialized for the first time. Swallow it so it
    // never escapes into a Qt slot.
    MITK_ERROR << "Unexpected error while listing nnInteractive models.";
    return {};
  }
}

mitk::nnInteractive::ModelCheckResult mitk::nnInteractive::CheckModelUpdate(PythonContext& context, const std::string& selectedModelId)
{
  ModelCheckResult result;
  result.CurrentId = selectedModelId;

  const std::string pyCommands =
    "nni_default_id = ''\n"
    "nni_model_check_ok = False\n"
    "try:\n"
    "    from nnInteractive.model_management import get_default_model_id\n"
    "    nni_default_id = str(get_default_model_id() or '')\n"
    "    nni_model_check_ok = True\n"
    "except Exception:\n"
    "    nni_default_id = ''\n"
    "    nni_model_check_ok = False\n";

  try
  {
    context.Execute(pyCommands);

    const bool ok = context.GetVariableAsBool("nni_model_check_ok").value_or(false);
    result.RecommendedId = context.GetVariableAsString("nni_default_id").value_or("");

    context.Execute("del nni_default_id, nni_model_check_ok\n");

    if (!ok || result.RecommendedId.empty())
      return result; // Status stays Unknown.

    // An empty selection means "use the recommended default", so it is by
    // definition up to date with whatever the default currently is.
    if (selectedModelId.empty() || selectedModelId == result.RecommendedId)
      result.Status = ModelUpdateStatus::UpToDate;
    else
      result.Status = ModelUpdateStatus::UpdateAvailable;
  }
  catch (const Exception& e)
  {
    MITK_WARN << "nnInteractive: could not check for a newer model checkpoint: " << e.GetDescription();
  }

  return result;
}
