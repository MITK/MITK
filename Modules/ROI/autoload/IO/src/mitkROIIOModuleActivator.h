/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkROIIOModuleActivator_h
#define mitkROIIOModuleActivator_h

#include <usModuleActivator.h>

#include <memory>
#include <vector>

namespace mitk
{
  class AbstractFileIO;

  /**
   * \brief Module activator for the ROI IO module.
   *
   * Registers the ROI file IO and serializer services when the module is loaded.
   */
  class ROIIOModuleActivator : public us::ModuleActivator
  {
  public:
    ROIIOModuleActivator() = default;
    ~ROIIOModuleActivator() override = default;

    ROIIOModuleActivator(const ROIIOModuleActivator&) = delete;
    ROIIOModuleActivator& operator=(const ROIIOModuleActivator&) = delete;

    /** \brief Register ROI IO and serializer services. */
    void Load(us::ModuleContext* context) override;

    /** \brief Unregister all ROI IO services. */
    void Unload(us::ModuleContext*) override;

  private:
    std::vector<std::shared_ptr<AbstractFileIO>> m_FileIOs;
  };
}

#endif
