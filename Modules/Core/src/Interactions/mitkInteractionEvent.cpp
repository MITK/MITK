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

#include "mitkInteractionEvent.h"
#include "mitkException.h"

const std::string mitk::InteractionEvent::KeyEsc = "Escape";
const std::string mitk::InteractionEvent::KeyEnter = "Enter";
const std::string mitk::InteractionEvent::KeyReturn = "Return";
const std::string mitk::InteractionEvent::KeyDelete = "Delete";
const std::string mitk::InteractionEvent::KeyArrowUp = "ArrowUp";
const std::string mitk::InteractionEvent::KeyArrowDown = "ArrowDown";
const std::string mitk::InteractionEvent::KeyArrowLeft = "ArrowLeft";
const std::string mitk::InteractionEvent::KeyArrowRight = "ArrowRight";

const std::string mitk::InteractionEvent::KeyF1 = "F1";
const std::string mitk::InteractionEvent::KeyF2 = "F2";
const std::string mitk::InteractionEvent::KeyF3 = "F3";
const std::string mitk::InteractionEvent::KeyF4 = "F4";
const std::string mitk::InteractionEvent::KeyF5 = "F5";
const std::string mitk::InteractionEvent::KeyF6 = "F6";
const std::string mitk::InteractionEvent::KeyF7 = "F7";
const std::string mitk::InteractionEvent::KeyF8 = "F8";
const std::string mitk::InteractionEvent::KeyF9 = "F9";
const std::string mitk::InteractionEvent::KeyF10 = "F10";
const std::string mitk::InteractionEvent::KeyF11 = "F11";
const std::string mitk::InteractionEvent::KeyF12 = "F12";

const std::string mitk::InteractionEvent::KeyPos1 = "Pos1";
const std::string mitk::InteractionEvent::KeyEnd = "End";
const std::string mitk::InteractionEvent::KeyInsert = "Insert";
const std::string mitk::InteractionEvent::KeyPageUp = "PageUp";
const std::string mitk::InteractionEvent::KeyPageDown = "PageDown";
const std::string mitk::InteractionEvent::KeySpace = "Space";

mitk::InteractionEvent::InteractionEvent(BaseRenderer *baseRenderer) : m_Sender(baseRenderer)
{
}

void mitk::InteractionEvent::SetSender(mitk::BaseRenderer *sender)
{
  m_Sender = sender;
}

mitk::BaseRenderer *mitk::InteractionEvent::GetSender() const
{
  return m_Sender;
}

bool mitk::InteractionEvent::IsEqual(const InteractionEvent &) const
{
  return true;
}

mitk::InteractionEvent::~InteractionEvent()
{
}

bool mitk::InteractionEvent::IsSuperClassOf(const InteractionEvent::Pointer &baseClass) const
{
  return (dynamic_cast<InteractionEvent *>(baseClass.GetPointer()) != nullptr);
}

bool mitk::operator==(const InteractionEvent &a, const InteractionEvent &b)
{
  return (typeid(a) == typeid(b) && a.IsEqual(b));
}

bool mitk::operator!=(const InteractionEvent &a, const InteractionEvent &b)
{
  return !(a == b);
}
