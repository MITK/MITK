/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkVoxTellInstall.h>

#include <mitkExceptionMacro.h>
#include <mitkPythonUtil.h>
#include <mitkTorchInstall.h>

#include <QFile>

#include <sstream>
#include <utility>

namespace
{
  // The install groups shared by a fresh install and an in-place upgrade.
  std::vector<mitk::PipInstallGroup> BuildGroups()
  {
    std::vector<mitk::PipInstallGroup> groups;

    groups.push_back(mitk::Torch::BuildInstallGroup());

    mitk::PipInstallGroup voxTellGroup;
    voxTellGroup.requirements = { std::string(mitk::VoxTell::DISTRIBUTION_NAME) + mitk::VoxTell::SupportedVersions().ToPipSpecifier() };
    groups.push_back(std::move(voxTellGroup));

    return groups;
  }

  // Defines mitk_download_from_hub(targets), which reports its progress.
  std::string ReadHubDownloadCode()
  {
    QFile file(":/HuggingFace/hub_download.py");

    if (!file.open(QIODevice::ReadOnly))
      mitkThrow() << "VoxTell: The resource with the download code could not be found.";

    return file.readAll().toStdString();
  }

  // At install time there is no Python context that could ask VoxTell for the
  // names of its files, so the step asks VoxTell itself.
  mitk::PostInstallStep BuildInstallDownloadStep(const std::string& displayName, const std::string& filesFunction)
  {
    mitk::PostInstallStep step;
    step.displayName = displayName;
    step.pythonCode = ReadHubDownloadCode() + "\n" + mitk::VoxTell::ReadHubFilesCode() + "\n"
      "mitk_download_from_hub(" + filesFunction + "())\n";
    step.optional = true;

    return step;
  }
}

mitk::PipInstallSpec mitk::VoxTell::BuildInstallSpec(ModelSource modelSource, const std::string& venvName, bool includeTextModel)
{
  mitk::PipInstallSpec spec;
  spec.name = "VoxTell";
  spec.venvName = venvName;
  spec.upgradePipFirst = true;
  spec.groups = BuildGroups();

  // VoxTell loads the embeddings of the prompts it knows with a local model as well.
  spec.postInstallSteps.push_back(BuildInstallDownloadStep("Download known prompts", "voxtell_prompt_list_files"));

  if (modelSource == ModelSource::HuggingFace)
    spec.postInstallSteps.push_back(BuildInstallDownloadStep("Download model", "voxtell_model_files"));

  if (includeTextModel)
    spec.postInstallSteps.push_back(BuildInstallDownloadStep("Download text model", "voxtell_text_model_files"));

  return spec;
}

mitk::PipInstallSpec mitk::VoxTell::BuildUpgradeSpec(const std::string& venvName)
{
  mitk::PipInstallSpec spec;
  spec.name = "VoxTell";
  spec.venvName = venvName;
  spec.upgradePipFirst = false;
  spec.groups = BuildGroups();

  // The venv already exists, so reuse the resolve-then-install engine with
  // --upgrade (and do not upgrade pip first). On Windows the torch group keeps
  // the CUDA wheel index, so a transitive torch bump from upgrading VoxTell
  // cannot pull a non-CUDA wheel from PyPI.
  for (auto& group : spec.groups)
    group.extraPipArgs.push_back("--upgrade");

  return spec;
}

mitk::PostInstallStep mitk::VoxTell::BuildDownloadStep(const std::string& displayName, const std::vector<RepoFiles>& files)
{
  std::ostringstream targets;
  targets << '[';

  for (const auto& repoFiles : files)
  {
    targets << '(' << PyQuote(repoFiles.RepoId) << ", [";

    for (const auto& pattern : repoFiles.Patterns)
      targets << PyQuote(pattern) << ", ";

    targets << "]), ";
  }

  targets << ']';

  mitk::PostInstallStep step;
  step.displayName = displayName;
  step.pythonCode = ReadHubDownloadCode() + "\nmitk_download_from_hub(" + targets.str() + ")\n";

  return step;
}
