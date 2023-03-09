/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkInteractionPositionEvent_h
#define mitkInteractionPositionEvent_h

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

#endif
