/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <usModuleActivator.h>

#include <memory>
#include <vector>

namespace mitk
{
  class AbstractFileIO;

  class ROIIOModuleActivator : public us::ModuleActivator
  {
  public:
    ROIIOModuleActivator() = default;
    ~ROIIOModuleActivator() override = default;

    ROIIOModuleActivator(const ROIIOModuleActivator&) = delete;
    ROIIOModuleActivator& operator=(const ROIIOModuleActivator&) = delete;

    void Load(us::ModuleContext* context) override;
    void Unload(us::ModuleContext*) override;

  private:
    std::vector<std::shared_ptr<AbstractFileIO>> m_FileIOs;
  };
}
