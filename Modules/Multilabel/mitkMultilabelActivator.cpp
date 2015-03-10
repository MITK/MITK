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

#include <usGetModuleContext.h>
#include <usModuleActivator.h>
#include <usModuleContext.h>
#include <usModule.h>

#include "mitkLabelSetImageIO.h"

namespace mitk
{

  /**
  \brief Registers services for multilabel module.
  */
  class MultilabelModuleActivator : public us::ModuleActivator
  {
    std::vector<AbstractFileIO*> m_FileIOs;
    
  public:

    void Load(us::ModuleContext* /*context*/)
    {
      m_FileIOs.push_back(new LabelSetImageIO());
    }

    void Unload(us::ModuleContext*)
    {
      for(std::vector<mitk::AbstractFileIO*>::iterator iter = m_FileIOs.begin(),
          endIter = m_FileIOs.end(); iter != endIter; ++iter)
      {
        delete *iter;
      }
    }
  };
}

US_EXPORT_MODULE_ACTIVATOR(mitk::MultilabelModuleActivator)
