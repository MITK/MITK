/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitknnInteractiveTool_h
#define mitknnInteractiveTool_h

#include "mitkSegWithPreviewTool.h"

namespace mitk
{
  class nnInteractiveTool : public SegWithPreviewTool
  {
  public:
    mitkClassMacro(nnInteractiveTool, SegWithPreviewTool);
    itkFactorylessNewMacro(Self);

    const char* GetName() const override;
    const char** GetXPM() const override;
    us::ModuleResource GetIconResource() const override;

  protected:
    nnInteractiveTool();
    ~nnInteractiveTool() override;

    void DoUpdatePreview(const Image* inputAtTimeStep, const Image* oldSegAtTimeStep, LabelSetImage* previewImage, TimeStepType timeStep) override;
  };
}

#endif
