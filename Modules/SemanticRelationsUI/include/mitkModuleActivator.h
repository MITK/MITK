/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef MITKMODULEACTIVATOR_H
#define MITKMODULEACTIVATOR_H

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
#endif // MITKMODULEACTIVATOR_H
