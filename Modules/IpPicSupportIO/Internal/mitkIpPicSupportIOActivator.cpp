/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <usModuleActivator.h>

#include "mitkPicFileReader.h"

class US_ABI_LOCAL mitkIpPicSupportIOActivator : public us::ModuleActivator
{
public:
  void Load(us::ModuleContext *) override { m_Reader = new mitk::PicFileReader(); }
  void Unload(us::ModuleContext *) override { delete m_Reader; }
private:
  mitk::IFileReader *m_Reader;
};

US_EXPORT_MODULE_ACTIVATOR(mitkIpPicSupportIOActivator)
