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

#ifndef MITKCPPRESTSDKQTACTIVATOR_H_
#define MITKCPPRESTSDKQTACTIVATOR_H_

#include <mitkRESTManagerQt.h>

#include <usModuleActivator.h>
#include <usModuleEvent.h> 
#include <usServiceRegistration.h> 
#include <usServiceTracker.h>

class MitkCppRestSdkQtActivator : public us::ModuleActivator
{

public:
  void Load(us::ModuleContext *context) override;
  void Unload(us::ModuleContext *) override;

private:
  std::unique_ptr<mitk::RESTManagerQt> m_RESTManagerQt;
};
#endif
