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

#ifndef MITKCPPRESTSDKACTIVATOR_H_
#define MITKCPPRESTSDKACTIVATOR_H_

#include <mitkRESTManager.h>

#include <usModuleActivator.h>
#include <usModuleEvent.h> 
#include <usServiceRegistration.h> 
#include <usServiceTracker.h>

class MitkCppRestSdkActivator : public us::ModuleActivator
{

public:
  void Load(us::ModuleContext *context) override;
  void Unload(us::ModuleContext *) override;

private:
  std::unique_ptr<mitk::RESTManager> m_RESTManager;
};
#endif
