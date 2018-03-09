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

#ifndef MITKINTERACTIONKEYEVENT_H_
#define MITKINTERACTIONKEYEVENT_H_

#include "itkObject.h"
#include "itkObjectFactory.h"
#include "mitkBaseRenderer.h"
#include "mitkCommon.h"
#include "mitkInteractionEvent.h"
#include "mitkInteractionEventConst.h"
#include "mitkInteractionPositionEvent.h"
#include <cstring>

#include <MitkCoreExports.h>

/*
 * Note: A Click with the MiddleButton is to be handled with MousePressEvents
 */
namespace mitk
{
  /**
   * \class InteractionKeyEvent
   * \brief Handles key events.
   * Takes a std::string for pressed key or special key description, mitk::ModifierKeys for modifiers
   * \ingroup Interaction.
   */

  class MITKCORE_EXPORT InteractionKeyEvent : public InteractionEvent
  {
  public:
    mitkClassMacro(InteractionKeyEvent, InteractionEvent)
      mitkNewMacro3Param(Self, BaseRenderer *, const std::string &, ModifierKeys)

        bool IsSuperClassOf(const InteractionEvent::Pointer &baseClass) const override;

    ModifierKeys GetModifiers() const;
    std::string GetKey() const;

  protected:
    InteractionKeyEvent(BaseRenderer *, const std::string &key, ModifierKeys modifiers);
    ~InteractionKeyEvent() override;

    bool IsEqual(const InteractionEvent &) const override;

  private:
    std::string m_Key;
    ModifierKeys m_Modifiers;
  };
} /* namespace mitk */

#endif /* MITKINTERACTIONKEYEVENT_H_ */
