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

#include <usModuleActivator.h>

#include "mitkPicFileReader.h"

class US_ABI_LOCAL mitkIpPicSupportIOActivator : public us::ModuleActivator
{
public:

  void Load(us::ModuleContext*)
  {
    m_Reader = new mitk::PicFileReader();
  }

  void Unload(us::ModuleContext* )
  {
    delete m_Reader;
  }

private:

  mitk::IFileReader* m_Reader;
};

US_EXPORT_MODULE_ACTIVATOR(mitkIpPicSupportIOActivator)
