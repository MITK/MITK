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

#ifndef MITKINTERACTIONPOSITIONEVENT_H_
#define MITKINTERACTIONPOSITIONEVENT_H_

#include "itkObject.h"
#include "itkObjectFactory.h"
#include "mitkCommon.h"
#include "mitkInteractionEvent.h"
#include <MitkExports.h>
#include <string>

namespace mitk
{

  class MITK_CORE_EXPORT InteractionPositionEvent : public InteractionEvent {

  public:
    const Point2D* GetMousePosition();
    const Point3D* GetWorldPosition();
    unsigned int GetModifiers();
    unsigned int GetButtonStates();

  protected:
    InteractionPositionEvent(BaseRenderer*, Point2D, unsigned int, unsigned int, std::string);
    virtual bool isEqual(InteractionEvent::Pointer) = 0;
    virtual ~InteractionPositionEvent();

  private:
    Point2D m_MousePosition;
    Point3D m_WorldPosition;
    unsigned int m_ButtonStates;
    unsigned int m_Modifiers;
  };

} /* namespace mitk */

#endif /* MITKINTERACTIONPOSITIONEVENT_H_ */
