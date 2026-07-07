/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkInternalEvent_h
#define mitkInternalEvent_h

#include <itkObject.h>
#include <itkObjectFactory.h>
#include <mitkBaseRenderer.h>
#include <mitkCommon.h>
#include <mitkInteractionEvent.h>
#include <mitkInteractionEventConst.h>
#include <string>

#include <MitkCoreExports.h>

namespace mitk
{
  /**
   * \brief Class to create events from within the application to signal about internal events.
   *
   * These events can target a specific DataInteractor, if this DataInteractor is specified in the constructor;
   * else this parameter is set to nullptr and the event is treated as a regular event.
   *
   * \sa InteractionEvent
   * \sa DataInteractor
   */
  class MITKCORE_EXPORT InternalEvent : public InteractionEvent
  {
  public:
    mitkClassMacro(InternalEvent, InteractionEvent);
    mitkNewMacro3Param(Self, BaseRenderer *, DataInteractor *, const std::string &);

    /**
     * \brief Get the signal name that identifies this internal event.
     *
     * \return The signal name string.
     */
    std::string GetSignalName() const;

    /**
     * \brief Get the DataInteractor that this event targets.
     *
     * \return Pointer to the target DataInteractor, or nullptr if no specific target.
     */
    DataInteractor *GetTargetInteractor() const;

    /**
     * \brief Check whether this event's class is a superclass of the given event.
     *
     * \param baseClass The event to check against.
     * \return True if this class is a superclass of \p baseClass.
     */
    bool IsSuperClassOf(const InteractionEvent::Pointer &baseClass) const override;

  protected:
    /**
     * \brief Construct an internal event.
     *
     * \param destInteractor The target DataInteractor, or nullptr for untargeted events.
     * \param signalName The name identifying this internal event signal.
     */
    InternalEvent(BaseRenderer *, DataInteractor *destInteractor, const std::string &signalName);
    ~InternalEvent() override;

    /**
     * \brief Compare this event to another InteractionEvent for equality.
     *
     * Two InternalEvents are equal if they have the same signal name.
     *
     * \return True if the events are equal.
     */
    bool IsEqual(const InteractionEvent &) const override;

  private:
    DataInteractor::Pointer m_DataInteractor;
    std::string m_SignalName;
  };
}
#endif
