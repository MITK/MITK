/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitknnInteractiveLassoTool_h
#define mitknnInteractiveLassoTool_h

#include <mitkAddContourTool.h>

namespace mitk
{
  class MITKSEGMENTATION_EXPORT LassoEvent : public itk::AnyEvent
  {
  public:
    using Self = LassoEvent;
    using Superclass = itk::AnyEvent;

    explicit LassoEvent(ContourModel* contour = nullptr);
    LassoEvent(const Self& other);
    ~LassoEvent() override;

    const char* GetEventName() const override;
    bool CheckEvent(const itk::EventObject* event) const override;
    itk::EventObject* MakeObject() const override;

    ContourModel* GetContour() const;

  private:
    ContourModel::Pointer m_Contour;
  };

  class MITKSEGMENTATION_EXPORT nnInteractiveLassoTool : public AddContourTool
  {
  public:
    mitkClassMacro(nnInteractiveLassoTool, AddContourTool)
    itkFactorylessNewMacro(Self)

    using Superclass::SetToolManager;

    us::ModuleResource GetCursorIconResource() const override;
    us::ModuleResource GetIconResource() const override;
    const char* GetName() const override;
    bool IsEligibleForAutoInit() const override;

    void Activate(const Color& color);
    void Deactivate();

  protected:
    nnInteractiveLassoTool();
    ~nnInteractiveLassoTool() override;

    void OnMouseReleased(StateMachineAction*, InteractionEvent*) override;
    void OnInvertLogic(StateMachineAction*, InteractionEvent*) override;
  };
}

#endif
