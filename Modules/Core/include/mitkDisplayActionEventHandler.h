/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkDisplayActionEventHandler_h
#define mitkDisplayActionEventHandler_h

#include <MitkCoreExports.h>

// mitk core
#include <mitkDisplayActionEventBroadcast.h>
#include <mitkDisplayActionEvents.h>
#include <mitkStdFunctionCommand.h>

namespace mitk
{
  /**
   * \brief Simplifies connecting display action event observers to a DisplayActionEventBroadcast.
   *
   * Use SetObservableBroadcast() to define which broadcast instance to observe,
   * then ConnectDisplayActionEvent() to register event-command pairs. Each
   * observer consists of a DisplayActionEvent (itkEventObject) and a
   * StdFunctionCommand (itkCommand) that is created internally from the
   * supplied std::function objects.
   *
   * \ingroup Interaction
   * \sa DisplayActionEventBroadcast DisplayActionEvents StdFunctionCommand
   */
  class MITKCORE_EXPORT DisplayActionEventHandler
  {
  public:

    /** \brief Tag type used to identify registered observers. */
    using OberserverTagType = unsigned long;

    /** \brief Destructor. Removes all registered observers. */
    virtual ~DisplayActionEventHandler();

    /**
     * \brief Set the DisplayActionEventBroadcast instance to observe.
     *
     * This class receives events from the given broadcast and triggers
     * the corresponding action functions.
     *
     * \post If the same broadcast was already set, nothing changes.
     * \post If a different broadcast was already set, all previously
     *       registered observers are removed.
     *
     * \param observableBroadcast The broadcast instance to observe.
     */
    void SetObservableBroadcast(DisplayActionEventBroadcast* observableBroadcast);

    /**
     * \brief Register an observer for a specific display action event.
     *
     * Creates a StdFunctionCommand from the given action and filter functions,
     * adds it as an observer to the broadcast, and stores its tag.
     *
     * \pre The observable broadcast must have been set via SetObservableBroadcast().
     * \throw mitk::Exception if the observable is null.
     *
     * \param displayActionEvent The event type to observe.
     * \param actionFunction     The function to execute when the event occurs.
     * \param filterFunction     Optional filter checked before execution (defaults to always true).
     * \return A tag identifying the newly created observer.
     */
    OberserverTagType ConnectDisplayActionEvent(const DisplayActionEvent& displayActionEvent,
      const StdFunctionCommand::ActionFunction& actionFunction,
      const StdFunctionCommand::FilterFunction& filterFunction = [](const itk::EventObject&) { return true; });

    /**
     * \brief Remove an observer identified by its tag.
     *
     * If the tag is not found in the internal list, nothing happens.
     *
     * \pre The observable broadcast must have been set.
     * \throw mitk::Exception if the observable is null.
     *
     * \param observerTag The tag identifying the observer to remove.
     */
    void DisconnectObserver(OberserverTagType observerTag);

    /**
     * \brief Get all currently registered observer tags.
     * \return A const reference to the vector of observer tags.
     */
    const std::vector<OberserverTagType>& GetAllObserverTags() { return m_ObserverTags; }

    /**
     * \brief Initialize pre-defined display actions and connect them to the broadcast.
     *
     * Removes all currently connected observers, then calls InitActionsImpl()
     * to let sub-classes define their specific set of actions.
     *
     * \pre The observable broadcast must have been set.
     * \throw mitk::Exception if the observable is null.
     *
     * \param prefixFilter Only react to / send changes to renderers whose name starts with this prefix.
     */
    void InitActions(std::string prefixFilter = "");

  protected:

    /**
     * \brief Override in sub-classes to define default display action connections.
     *
     * \param prefixFilter Only react to / send changes to renderers whose name starts with this prefix.
     */
    virtual void InitActionsImpl(const std::string& prefixFilter = "");

    WeakPointer<DisplayActionEventBroadcast> m_ObservableBroadcast;
    std::vector<OberserverTagType> m_ObserverTags;

  };

} // end namespace mitk

#endif
