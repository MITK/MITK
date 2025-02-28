/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitknnInteractiveScribbleTool.h>
#include <mitkToolManager.h>

namespace
{
  void InitializeVolume(mitk::Image* image)
  {
    size_t numPixels = 1;

    for (int i = 0; i < 3; ++i)
      numPixels *= image->GetDimension(i);

    auto data = new mitk::Label::PixelType[numPixels];
    std::memset(data, 0, numPixels * sizeof(mitk::Label::PixelType));

    image->SetImportVolume(data, 0, 0, mitk::Image::ReferenceMemory);
  }
}

mitk::ScribbleEvent::ScribbleEvent(Image* mask)
  : m_Mask(mask)
{
}

mitk::ScribbleEvent::ScribbleEvent(const Self& other)
  : Superclass(other),
    m_Mask(other.m_Mask)
{
}

mitk::ScribbleEvent::~ScribbleEvent() = default;

const char* mitk::ScribbleEvent::GetEventName() const
{
  return "ScribbleEvent";
}

bool mitk::ScribbleEvent::CheckEvent(const itk::EventObject* event) const
{
  return dynamic_cast<const Self*>(event) != nullptr;
}

itk::EventObject* mitk::ScribbleEvent::MakeObject() const
{
  return new Self(*this);
}

mitk::Image* mitk::ScribbleEvent::GetMask() const
{
  return m_Mask;
}

mitk::nnInteractiveScribbleTool::nnInteractiveScribbleTool()
{
  this->DisableContourMarkers();
  this->SetSize(3);
  this->Disable();
}

mitk::nnInteractiveScribbleTool::~nnInteractiveScribbleTool()
{
}

us::ModuleResource mitk::nnInteractiveScribbleTool::GetCursorIconResource() const
{
  return us::ModuleResource();
}

us::ModuleResource mitk::nnInteractiveScribbleTool::GetIconResource() const
{
  return us::ModuleResource();
}

const char* mitk::nnInteractiveScribbleTool::GetName() const
{
  return "Scribble";
}

bool mitk::nnInteractiveScribbleTool::IsEligibleForAutoInit() const
{
  return false;
}

void mitk::nnInteractiveScribbleTool::Activate(const Color& color)
{
  this->SetEnable3DInterpolation(false);
  this->Activated();
  this->Enable();

  this->SetFeedbackContourColor(color);

  m_PaintingNode->SetColor(color);
  m_PaintingNode->SetBoolProperty("outline binary", false);
}

void mitk::nnInteractiveScribbleTool::Deactivate()
{
  this->Disable();
  this->Deactivated();
  this->SetEnable3DInterpolation(true);
}

int mitk::nnInteractiveScribbleTool::GetFillValue() const
{
  return 1;
}

void mitk::nnInteractiveScribbleTool::OnMouseReleased(StateMachineAction* action, InteractionEvent* event)
{
  auto mask = Image::New();
  mask->Initialize(this->GetWorkingData());
  InitializeVolume(mask);
  this->WriteSliceToVolume(mask, m_CurrentPlane, m_PaintingSlice, 0, false);

  Superclass::OnMouseReleased(action, event);
  this->InvokeEvent(ScribbleEvent(mask));
}

void mitk::nnInteractiveScribbleTool::OnInvertLogic(StateMachineAction*, InteractionEvent*)
{
}
