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

#ifndef MITKCESTIOActivator_H
#define MITKCESTIOActivator_H

#include <mitkCustomMimeType.h>

#include <usModuleActivator.h>
#include <usServiceEvent.h>

#include <memory>

namespace mitk
{
  struct IFileReader;
  class IDICOMTagsOfInterest;

  class CESTIOActivator : public us::ModuleActivator
  {
  public:
    void Load(us::ModuleContext *context) override;
    void Unload(us::ModuleContext *context) override;

  private:
    std::unique_ptr<IFileReader> m_CESTDICOMReader;
    us::ModuleContext *mitkContext;
    std::vector<mitk::CustomMimeType *> m_MimeTypes;
  };
}

#endif // MITKCESTIOActivator_H
