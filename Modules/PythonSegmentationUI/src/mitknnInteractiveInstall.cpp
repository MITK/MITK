/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitknnInteractiveInstall.h>

#include <mitkIPreferences.h>
#include <mitkPythonUtil.h>
#include <mitknnInteractiveVersion.h>

#include <utility>

namespace
{
  std::string VersionRange()
  {
    return std::string(">=") + mitk::nnInteractive::MINIMUM_VERSION
      + ",<" + mitk::nnInteractive::MAXIMUM_VERSION_EXCLUSIVE;
  }
}

mitk::PipInstallSpec mitk::nnInteractive::BuildInstallSpec(mitk::IPreferences* prefs, const std::string& venvName, bool clientOnly)
{
  mitk::PipInstallSpec spec;
  spec.name = "nnInteractive";
  spec.venvName = venvName;
  spec.upgradePipFirst = true;

  const auto versionRange = VersionRange();

  if (clientOnly)
  {
    mitk::PipInstallGroup clientGroup;
    // The version check imports 'packaging'; the full install gets it via torch,
    // so the client-only group must request it explicitly.
    clientGroup.requirements = { "nninteractive-client" + versionRange, "packaging" };
    spec.groups.push_back(std::move(clientGroup));
    return spec;
  }

  // PyTorch needs a CUDA-specific index URL on Windows; other platforms use the
  // default PyPI index. (cu128: with CUDA 12.9 our lowest supported GPU arch
  // hits "no kernel image is available".)
#if defined(_WIN32)
  const std::string cudaIndexUrl = "https://download.pytorch.org/whl/cu128";
#else
  const std::string cudaIndexUrl;
#endif

  mitk::PipInstallGroup torchGroup;
  torchGroup.requirements = { "torch>=2.8.0,<2.9.0", "torchvision>=0.23.0,<1.0.0" };
  torchGroup.indexUrl = cudaIndexUrl;
  spec.groups.push_back(std::move(torchGroup));

  mitk::PipInstallGroup nnInteractiveGroup;
  nnInteractiveGroup.requirements = { "nninteractive" + versionRange };
  spec.groups.push_back(std::move(nnInteractiveGroup));

  // Pre-download the model checkpoint via nnInteractive's model management so the
  // first local StartSession() does not surprise the user with a silent
  // multi-minute download. Skipped when a local checkpoint folder is configured.
  // Optional: a failure here is non-fatal, since ConstructLocalSession() calls
  // ensure_model_available() again as a fallback.
  if (prefs != nullptr && prefs->Get("nnInteractive/modelSource", "huggingface") != "local")
  {
    const auto modelCheckpoint = prefs->Get("nnInteractive/modelCheckpoint", "");
    const std::string ensureArg = modelCheckpoint.empty()
      ? "get_default_model_id()"
      : mitk::PyQuote(modelCheckpoint);

    mitk::PostInstallStep step;
    step.displayName = "Download model weights";
    step.pythonCode =
      "from nnInteractive.model_management import ensure_model_available, get_default_model_id\n"
      "ensure_model_available(" + ensureArg + ")\n";
    step.optional = true;
    spec.postInstallSteps.push_back(std::move(step));
  }

  return spec;
}

mitk::PipInstallSpec mitk::nnInteractive::BuildUpgradeSpec(const std::string& venvName, bool clientOnly)
{
  mitk::PipInstallSpec spec;
  spec.name = "nnInteractive";
  spec.venvName = venvName;
  spec.upgradePipFirst = false;

  const auto versionRange = VersionRange();

  if (clientOnly)
  {
    mitk::PipInstallGroup clientGroup;
    // Keep 'packaging' present after an upgrade (see BuildInstallSpec).
    clientGroup.requirements = { "nninteractive-client" + versionRange, "packaging" };
    clientGroup.extraPipArgs = { "--upgrade" };
    spec.groups.push_back(std::move(clientGroup));
    return spec;
  }

  // Upgrade the torch group first (with its CUDA index) so a transitive torch
  // bump from upgrading nnInteractive cannot pull a non-CUDA wheel from PyPI on
  // Windows; the version pin keeps torch within its supported range.
#if defined(_WIN32)
  const std::string cudaIndexUrl = "https://download.pytorch.org/whl/cu128";
#else
  const std::string cudaIndexUrl;
#endif

  mitk::PipInstallGroup torchGroup;
  torchGroup.requirements = { "torch>=2.8.0,<2.9.0", "torchvision>=0.23.0,<1.0.0" };
  torchGroup.indexUrl = cudaIndexUrl;
  torchGroup.extraPipArgs = { "--upgrade" };
  spec.groups.push_back(std::move(torchGroup));

  mitk::PipInstallGroup nnInteractiveGroup;
  nnInteractiveGroup.requirements = { "nninteractive" + versionRange };
  nnInteractiveGroup.extraPipArgs = { "--upgrade" };
  spec.groups.push_back(std::move(nnInteractiveGroup));

  return spec;
}
