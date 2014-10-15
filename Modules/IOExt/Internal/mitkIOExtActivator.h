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

#ifndef MITKIOEXTACTIVATOR_H
#define MITKIOEXTACTIVATOR_H

#include <usModuleActivator.h>

#include <memory>

namespace mitk {

struct IFileReader;

class IOExtActivator : public us::ModuleActivator
{
public:

  void Load(us::ModuleContext*context);
  void Unload(us::ModuleContext* context);

private:

  std::auto_ptr<IFileReader> m_SceneReader;
};

}

#endif // MITKIOEXTACTIVATOR_H
