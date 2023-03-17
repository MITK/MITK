/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkModuleActivator_h
#define mitkModuleActivator_h

// Micro Services
#include <usModuleActivator.h>
#include <usModuleEvent.h>
#include <usServiceRegistration.h>
#include <usServiceTracker.h>

#include <memory>

// qt widgets module
#include "mitkIDataStorageInspectorProvider.h"

namespace mitk
{
  /*
   * This is the module activator for the "SemanticRelationsUI" module.
   */
  class SemanticRelationsUIActivator : public us::ModuleActivator
  {
  public:
    void Load(us::ModuleContext* context) override;
    void Unload(us::ModuleContext* context) override;

  private:
    std::unique_ptr<mitk::IDataStorageInspectorProvider> m_PatientTableInspector;
  };
}
#endif
