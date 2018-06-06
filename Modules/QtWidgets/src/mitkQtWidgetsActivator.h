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

#ifndef MITKQTWIDGETSACTIVATOR_H_
#define MITKQTWIDGETSACTIVATOR_H_

// Micro Services
#include <usModuleActivator.h>
#include <usModuleEvent.h>
#include <usServiceRegistration.h>
#include <usServiceTracker.h>

#include <memory>

#include "mitkIDataStorageInspectorProvider.h"

/*
 * This is the module activator for the "QtWidgets" module.
 */
class MitkQtWidgetsActivator : public us::ModuleActivator
{
public:
  void Load(us::ModuleContext *context) override;
  void Unload(us::ModuleContext *) override;

private:
  std::unique_ptr<mitk::IDataStorageInspectorProvider> m_TreeInspector;
  std::unique_ptr<mitk::IDataStorageInspectorProvider> m_ListInspector;
};

#endif // MITKCOREACTIVATOR_H_
