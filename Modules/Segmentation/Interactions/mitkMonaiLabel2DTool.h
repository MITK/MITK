/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef MITKMONAILABEL2DTOOL_H
#define MITKMONAILABEL2DTOOL_H

#include "mitkSegWithPreviewTool.h"
#include "mitkMonaiLabelTool.h"
#include <MitkSegmentationExports.h>
#include <memory>
#include <unordered_map>
#include <set>
#include <httplib.h>
#include <nlohmann/json.hpp>
#include "mitkMonaiLabelTool.h"


namespace us
{
  class ModuleResource;
}

namespace mitk
{
  class MITKSEGMENTATION_EXPORT MonaiLabel2DTool : public MonaiLabelTool
  {
  public:
    mitkClassMacro(MonaiLabel2DTool, MonaiLabelTool);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    const char *GetName() const override;
    const char **GetXPM() const override;
    us::ModuleResource GetIconResource() const override;

    void Activated() override;
    
  protected:
    MonaiLabel2DTool();
    ~MonaiLabel2DTool() = default;
   
  };
}
#endif
