/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkScribbleTool_h
#define mitkScribbleTool_h

#include <mitkDrawPaintbrushTool.h>

namespace mitk
{
  class ScribbleTool : public DrawPaintbrushTool
  {
  public:
    mitkClassMacro(ScribbleTool, DrawPaintbrushTool)
    itkFactorylessNewMacro(Self)

    using Superclass::SetToolManager;

    us::ModuleResource GetCursorIconResource() const override;
    us::ModuleResource GetIconResource() const override;
    const char* GetName() const override;
    bool IsEligibleForAutoInit() const override;

    void Activate(const Color& color);
    void Deactivate();

  protected:
    ScribbleTool();
    ~ScribbleTool() override;

    void OnInvertLogic(StateMachineAction*, InteractionEvent*) override;
  };
}

#endif
