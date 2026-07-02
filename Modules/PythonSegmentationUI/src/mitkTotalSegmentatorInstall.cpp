/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkTotalSegmentatorInstall.h>

#include <utility>

namespace
{
  // nnunetv2 version validated against REQUIRED_VERSION. See BuildGroups() for why
  // it is pinned; bump it together with mitk::TotalSegmentator::REQUIRED_VERSION.
  constexpr const char* NNUNETV2_VERSION = "2.8.1";

  // PyTorch needs a CUDA-specific index URL on Windows; other platforms use the
  // default PyPI index. Kept identical to the nnInteractive install (cu128: with
  // CUDA 12.9 our lowest supported GPU arch, Pascal / GeForce 10-series, hits
  // "no kernel image is available"), so both tools pull a Pascal-capable build.
  std::string CudaIndexUrl()
  {
#if defined(_WIN32)
    return "https://download.pytorch.org/whl/cu128";
#else
    return {};
#endif
  }

  // Dumps a { task: { labelId: labelName } } map to <venv>/mitk_totalseg_tasks.json
  // so the tool names its output labels from structured data instead of parsing the
  // package source. Prefers the machine-readable registry (newer TotalSegmentator)
  // and falls back to map_to_binary.class_map. Every lookup is guarded so one bad
  // task cannot abort the dump, and the file is written atomically. Failure is
  // non-fatal: the tool auto-names labels when the file is missing.
  std::string TaskDumpCode()
  {
    return R"PY(
import json, os, sys

data = {}

def _add(name, classes):
    try:
        data[str(name)] = {str(int(k)): str(v) for k, v in classes.items()}
    except Exception:
        pass

try:
    import totalsegmentator.registry as _reg
    for _t in _reg.TASKS:
        try:
            _add(_t, _reg.get_task_classes(_t))
        except Exception:
            pass
except Exception:
    try:
        from totalsegmentator.map_to_binary import class_map
        for _t, _m in class_map.items():
            _add(_t, _m)
    except Exception:
        data = {}

_path = os.path.join(sys.prefix, "mitk_totalseg_tasks.json")
_tmp = _path + ".tmp"
with open(_tmp, "w", encoding="utf-8") as _f:
    json.dump(data, _f)
os.replace(_tmp, _path)
)PY";
  }

  std::vector<mitk::PipInstallGroup> BuildGroups()
  {
    std::vector<mitk::PipInstallGroup> groups;

    // torchvision must be pinned together with torch (and installed from the same
    // CUDA index), even though TotalSegmentator does not use it directly: its
    // transitive dependency timm pulls torchvision, and each torchvision release
    // hard-pins a matching torch (e.g. torchvision 0.27.1 requires torch 2.12.1).
    // Without pinning it here, the TotalSegmentator resolve grabs the latest
    // torchvision and drags torch off the CUDA 2.8 wheel onto a non-CUDA PyPI
    // build. The torch upper bound constrains torchvision to its 0.23.x pair.
    mitk::PipInstallGroup torchGroup;
    torchGroup.requirements = { "torch>=2.8.0,<2.9.0", "torchvision>=0.23.0,<1.0.0" };
    torchGroup.indexUrl = CudaIndexUrl();
    groups.push_back(std::move(torchGroup));

    // TotalSegmentator leaves its own nnunetv2 dependency unbounded
    // ("nnunetv2>=2.3.1") and has never fixed this upstream. New nnunetv2 releases
    // have repeatedly broken the environment (hence the recurring "Totalseg
    // dependency update" pins in MITK's history), and nnunetv2 is what drives the
    // resolved torch version. Pin it to the version validated against the
    // TotalSegmentator release above so a future nnunetv2 cannot silently change
    // torch or the wider tree. Bump NNUNETV2_VERSION together with REQUIRED_VERSION.
    mitk::PipInstallGroup totalSegGroup;
    totalSegGroup.requirements = {
      std::string("TotalSegmentator==") + mitk::TotalSegmentator::REQUIRED_VERSION,
      std::string("nnunetv2==") + NNUNETV2_VERSION
    };
    groups.push_back(std::move(totalSegGroup));

    return groups;
  }

  mitk::PostInstallStep TaskDumpStep()
  {
    mitk::PostInstallStep step;
    step.displayName = "Prepare task list";
    step.pythonCode = TaskDumpCode();
    step.optional = true;
    return step;
  }

  // TotalSegmentator sends anonymous usage statistics to its own server by
  // default. Opt out on behalf of the user by clearing the config flag right
  // after install. Non-fatal: a failure here just leaves the default behaviour.
  mitk::PostInstallStep DisableUsageStatsStep()
  {
    mitk::PostInstallStep step;
    step.displayName = "Disable usage statistics";
    step.pythonCode =
      "try:\n"
      "    from totalsegmentator.config import set_config_key\n"
      "    set_config_key('send_usage_stats', False)\n"
      "except Exception:\n"
      "    pass\n";
    step.optional = true;
    return step;
  }
}

mitk::PipInstallSpec mitk::TotalSegmentator::BuildInstallSpec(const std::string& venvName)
{
  mitk::PipInstallSpec spec;
  spec.name = "TotalSegmentator";
  spec.venvName = venvName;
  spec.upgradePipFirst = true;
  spec.groups = BuildGroups();
  spec.postInstallSteps.push_back(TaskDumpStep());
  spec.postInstallSteps.push_back(DisableUsageStatsStep());

  return spec;
}
