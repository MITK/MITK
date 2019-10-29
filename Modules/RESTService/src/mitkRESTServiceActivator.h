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
