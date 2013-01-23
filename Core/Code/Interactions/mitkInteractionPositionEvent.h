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
#include "mitkInteractionEventConst.h"
#include <MitkExports.h>
#include <string>

namespace mitk
{
/**
 * \class InteractionPositionEvent
 *
 * \brief Super class for all position events.
 *
 * This class is instantiated with a BaseRenderer and the 2D pointer position relative to the renderer,
 * the object then queries the Renderer for 3D world coordinates and supplies them to deriving classes.
 *
 */

  class MITK_CORE_EXPORT InteractionPositionEvent : public InteractionEvent {

  public:

    mitkClassMacro(InteractionPositionEvent,InteractionEvent);
    mitkNewMacro3Param(Self, BaseRenderer*, Point2D , std::string);

    Point2D GetPointerPositionOnScreen();
    Point3D GetPositionInWorld();
    virtual bool IsSuperClassOf(InteractionEvent::Pointer baseClass);

  protected:
    InteractionPositionEvent(BaseRenderer* baseRenderer, Point2D mousePosition, std::string eventClass);
    virtual bool isEqual(InteractionEvent::Pointer);
    virtual ~InteractionPositionEvent();


  private:
    Point2D m_PointerPosition;
    Point3D m_WorldPosition;
  };

} /* namespace mitk */

#endif /* MITKINTERACTIONPOSITIONEVENT_H_ */
