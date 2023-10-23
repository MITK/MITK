/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef MITKMONAILABEL3DTOOL_H
#define MITKMONAILABEL3DTOOL_H

#include "mitkMonaiLabelTool.h"
#include <MitkSegmentationExports.h>

namespace us
{
  class ModuleResource;
}

namespace mitk
{

  class MITKSEGMENTATION_EXPORT MonaiLabel3DTool : public MonaiLabelTool // SegWithPreviewTool
  {
  public:
    mitkClassMacro(MonaiLabel3DTool, MonaiLabelTool);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    const char *GetName() const override;
    const char **GetXPM() const override;
    us::ModuleResource GetIconResource() const override;
    void Activated() override;
    void OnAddPositivePoint(StateMachineAction *, InteractionEvent *) override;
    void OnAddNegativePoint(StateMachineAction *, InteractionEvent *) override;
    void WriteImage(const Image *, std::string &) override;
    void WriteBackResults(LabelSetImage *, LabelSetImage *, TimeStepType) override;

  protected:
    MonaiLabel3DTool();
    ~MonaiLabel3DTool() = default;
  };
} // namespace mitk
#endif
