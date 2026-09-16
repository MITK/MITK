/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitknnInteractiveInstall.h>

#include <mitkIPreferences.h>
#include <mitkPythonHelper.h>
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

  // PyTorch needs a CUDA-specific index URL on Windows; other platforms use the
  // default PyPI index. (cu128: with CUDA 12.9 our lowest supported GPU arch
  // hits "no kernel image is available".)
  std::string CudaIndexUrl()
  {
#if defined(_WIN32)
    return "https://download.pytorch.org/whl/cu128";
#else
    return {};
#endif
  }

  // torch 2.8 is the last release whose CUDA builds contain Pascal kernels
  // (sm_61, GeForce 10 series), and it publishes no wheels for CPython 3.14 or
  // newer. There the oldest usable release is 2.10 -- 2.9 has a blocking defect
  // for us -- which no longer runs on Pascal, so the minimum GPU rises to
  // Turing. Keep in sync with the compute capability floor in
  // mitknnInteractiveTool.cpp and the GPU list in QmitknnInteractiveToolGUI.cpp.
  //
  // torchvision has to be pinned alongside torch and installed from the same
  // index, because every torchvision release hard-pins one torch version; the
  // torch upper bound is what keeps it on the matching minor.
  std::vector<std::string> TorchRequirements()
  {
    if constexpr (mitk::PythonHelper::VERSION_MINOR >= 14)
      return { "torch>=2.10.0,<2.11.0", "torchvision>=0.25.0,<1.0.0" };
    else
      return { "torch>=2.8.0,<2.9.0", "torchvision>=0.23.0,<1.0.0" };
  }

  // The install groups shared by a fresh install and an in-place upgrade. Client
  // only installs the lightweight, torch-free nninteractive-client; full mode
  // installs PyTorch (from the CUDA wheel index on Windows) and nnInteractive.
  // The version check imports 'packaging'; the full install gets it via torch,
  // so the client-only group must request it explicitly.
  std::vector<mitk::PipInstallGroup> BuildGroups(bool clientOnly)
  {
    const auto versionRange = VersionRange();

    std::vector<mitk::PipInstallGroup> groups;

    if (clientOnly)
    {
      mitk::PipInstallGroup clientGroup;
      clientGroup.requirements = { "nninteractive-client" + versionRange, "packaging" };
      groups.push_back(std::move(clientGroup));
      return groups;
    }

    mitk::PipInstallGroup torchGroup;
    torchGroup.requirements = TorchRequirements();
    torchGroup.indexUrl = CudaIndexUrl();
    groups.push_back(std::move(torchGroup));

    mitk::PipInstallGroup nnInteractiveGroup;
    nnInteractiveGroup.requirements = { "nninteractive" + versionRange };
    groups.push_back(std::move(nnInteractiveGroup));

    return groups;
  }
}

mitk::PipInstallSpec mitk::nnInteractive::BuildInstallSpec(mitk::IPreferences* prefs, const std::string& venvName, bool clientOnly)
{
  mitk::PipInstallSpec spec;
  spec.name = "nnInteractive";
  spec.venvName = venvName;
  spec.upgradePipFirst = true;
  spec.groups = BuildGroups(clientOnly);

  // Pre-download the model checkpoint via nnInteractive's model management so the
  // first local StartSession() does not surprise the user with a silent
  // multi-minute download. Skipped for a client-only install (no local inference)
  // and when a local checkpoint folder is configured. Optional: a failure here is
  // non-fatal, since ConstructLocalSession() calls ensure_model_available() again
  // as a fallback.
  if (!clientOnly && prefs != nullptr && prefs->Get("nnInteractive/modelSource", "huggingface") != "local")
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
  spec.groups = BuildGroups(clientOnly);

  // The venv already exists, so reuse the resolve-then-install engine with
  // --upgrade (and do not upgrade pip first). On Windows the torch group keeps
  // the CUDA wheel index set in BuildGroups, so a transitive torch bump from
  // upgrading nnInteractive cannot pull a non-CUDA wheel from PyPI.
  for (auto& group : spec.groups)
    group.extraPipArgs.push_back("--upgrade");

  return spec;
}
