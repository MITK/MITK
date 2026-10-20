/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkTotalSegmentatorInstall.h>

#include <mitkTotalSegmentatorTool.h>
#include <mitkTorchInstall.h>

#include <utility>

namespace
{
  std::vector<mitk::PipInstallGroup> BuildGroups()
  {
    std::vector<mitk::PipInstallGroup> groups;

    groups.push_back(mitk::Torch::BuildInstallGroup());

    mitk::PipInstallGroup totalSegGroup;
    totalSegGroup.requirements = {
      std::string("TotalSegmentator==") + mitk::TotalSegmentator::REQUIRED_VERSION,
      std::string("nnunetv2==") + mitk::TotalSegmentator::NNUNETV2_VERSION
    };
    groups.push_back(std::move(totalSegGroup));

    return groups;
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
  spec.postInstallSteps.push_back(DisableUsageStatsStep());

  return spec;
}
