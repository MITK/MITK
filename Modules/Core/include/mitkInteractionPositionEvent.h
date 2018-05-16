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
#include <MitkCoreExports.h>
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

  class MITKCORE_EXPORT InteractionPositionEvent : public InteractionEvent
  {
  public:
    mitkClassMacro(InteractionPositionEvent, InteractionEvent);
    mitkNewMacro2Param(Self, BaseRenderer *, const Point2D &);

    Point2D GetPointerPositionOnScreen() const;
    Point3D GetPositionInWorld() const;

    bool IsSuperClassOf(const InteractionEvent::Pointer &baseClass) const override;

  protected:
    InteractionPositionEvent(BaseRenderer *baseRenderer, const Point2D &mousePosition);
    ~InteractionPositionEvent() override;

    bool IsEqual(const InteractionEvent &) const override;

  private:
    const Point2D m_PointerPosition;
  };

} /* namespace mitk */

#endif /* MITKINTERACTIONPOSITIONEVENT_H_ */
