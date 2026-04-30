/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkInteractionPositionEvent_h
#define mitkInteractionPositionEvent_h

#include <itkObject.h>
#include <itkObjectFactory.h>
#include <mitkCommon.h>
#include <mitkInteractionEvent.h>
#include <mitkInteractionEventConst.h>
#include <MitkCoreExports.h>
#include <string>

namespace mitk
{
  /**
   * \class InteractionPositionEvent
   *
   * \brief Super class for all position-aware interaction events.
   *
   * This class stores the 2D pointer position (in display coordinates) relative to
   * the BaseRenderer. On demand, it converts this to a 3D world coordinate via the
   * renderer's DisplayToWorld() method.
   *
   * \sa InteractionEvent
   * \sa InteractionKeyEvent
   * \ingroup Interaction
   */

  class MITKCORE_EXPORT InteractionPositionEvent : public InteractionEvent
  {
  public:
    mitkClassMacro(InteractionPositionEvent, InteractionEvent);
    mitkNewMacro2Param(Self, BaseRenderer *, const Point2D &);

    /**
     * \brief Get the pointer position in display (screen) coordinates.
     * \return The 2D pointer position in pixels.
     */
    Point2D GetPointerPositionOnScreen() const;

    /**
     * \brief Get the pointer position converted to 3D world coordinates.
     *
     * The conversion is performed via the sender renderer's DisplayToWorld() method.
     *
     * \return The 3D world position corresponding to the pointer position.
     */
    Point3D GetPositionInWorld() const;

    /**
     * \brief Check if the provided event is an InteractionPositionEvent or derived.
     * \param[in] baseClass The event to check.
     * \return true if baseClass is an InteractionPositionEvent or derived from it.
     */
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
