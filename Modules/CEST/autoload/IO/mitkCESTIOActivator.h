/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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
    std::vector<mitk::CustomMimeType *> m_MimeTypes;
  };
}

#endif // MITKCESTIOActivator_H
