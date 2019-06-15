#pragma once

#include "mitkDrawPaintbrushTool.xpm"

#include <MitkSegmentationExports.h>

#include <usModule.h>
#include <usModuleResource.h>
#include <usGetModuleContext.h>
#include <usModuleContext.h>

#include "mitkContourTool.h"

namespace mitk
{
class MITKSEGMENTATION_EXPORT ResectionTool : public ContourTool
{
public:
  enum ResectionType {
    INSIDE,
    OUTSIDE,
    DIVIDE,
  };

  enum ResectionState {
    NO_CONTOUR,
    CONTOUR_AVAILABLE,
  };

  Message1<ResectionState> stateChanged;

  mitkClassMacro(ResectionTool, ContourTool);
  itkFactorylessNewMacro(Self);
  itkCloneMacro(Self);

  virtual const char** GetXPM() const override;
  virtual us::ModuleResource GetCursorIconResource() const override;
  us::ModuleResource GetIconResource() const override;
  virtual const char* GetName() const override;

  void Resect(ResectionType type = ResectionType::INSIDE);

  ResectionState GetState();

protected:
  class CommandProgressUpdate : public itk::Command
  {
  public:
    using Self = CommandProgressUpdate;
    using Superclass = itk::Command;
    using Pointer = itk::SmartPointer<Self>;

    itkNewMacro(Self);

    void Execute(itk::Object* caller, const itk::EventObject& event) override
    {
      Execute((const itk::Object*) caller, event);
    }

    void Execute(const itk::Object* caller, const itk::EventObject& event) override;

    void setNumberOfSteps(int steps)
    {
      m_NumberOfSteps = steps;
    }

  protected:
    CommandProgressUpdate() {};

    int m_LastStep = 0;
    int m_NumberOfSteps = 100;
  };

  ResectionState m_State;

  ResectionTool();
  virtual ~ResectionTool();

  virtual void OnMousePressed(StateMachineAction*, InteractionEvent* interactionEvent) override;
  virtual void OnMouseReleased(StateMachineAction*, InteractionEvent* interactionEvent) override;

  virtual void Activated() override;
};
}

