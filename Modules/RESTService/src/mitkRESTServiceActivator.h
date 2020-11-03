/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkRESTServiceActivator_h
#define mitkRESTServiceActivator_h

#include <mitkRESTManager.h>
#include <usModuleActivator.h>

class MitkRESTServiceActivator : public us::ModuleActivator
{
public:
  void Load(us::ModuleContext *context) override;
  void Unload(us::ModuleContext *context) override;

private:
  std::unique_ptr<mitk::RESTManager> m_RESTManager;
};

#endif
