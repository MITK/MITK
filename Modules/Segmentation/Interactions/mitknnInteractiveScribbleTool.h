/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitknnInteractiveScribbleTool_h
#define mitknnInteractiveScribbleTool_h

#include <mitkDrawPaintbrushTool.h>

namespace mitk
{
  class MITKSEGMENTATION_EXPORT ScribbleEvent : public itk::AnyEvent
  {
  public:
    using Self = ScribbleEvent;
    using Superclass = itk::AnyEvent;

    explicit ScribbleEvent(Image* mask = nullptr);
    ScribbleEvent(const Self& other);
    ~ScribbleEvent() override;

    const char* GetEventName() const override;
    bool CheckEvent(const itk::EventObject* event) const override;
    itk::EventObject* MakeObject() const override;

    Image* GetMask() const;

  private:
    Image::Pointer m_Mask;
  };

  class MITKSEGMENTATION_EXPORT nnInteractiveScribbleTool : public DrawPaintbrushTool
  {
  public:
    mitkClassMacro(nnInteractiveScribbleTool, DrawPaintbrushTool)
    itkFactorylessNewMacro(Self)

    using Superclass::SetToolManager;

    us::ModuleResource GetCursorIconResource() const override;
    us::ModuleResource GetIconResource() const override;
    const char* GetName() const override;
    bool IsEligibleForAutoInit() const override;

    void Activate(const Color& color);
    void Deactivate();

  protected:
    nnInteractiveScribbleTool();
    ~nnInteractiveScribbleTool() override;

    int GetFillValue() const override;
    void OnMouseReleased(StateMachineAction*, InteractionEvent*) override;
    void OnInvertLogic(StateMachineAction*, InteractionEvent*) override;
  };
}

#endif
