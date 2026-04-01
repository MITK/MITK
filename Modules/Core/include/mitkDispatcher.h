/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkDispatcher_h
#define mitkDispatcher_h

#include <itkLightObject.h>
#include <itkObjectFactory.h>
#include <mitkCommon.h>
#include <mitkDataInteractor.h>
#include <mitkDataNode.h>
#include <usServiceTracker.h>
#include <MitkCoreExports.h>
#include <list>
#include <mitkWeakPointer.h>

namespace mitk
{
  class InternalEvent;
  class InteractionEvent;
  struct InteractionEventObserver;

  /**
   * \class Dispatcher
   * \brief Central event distribution hub for interaction events.
   *
   * The Dispatcher receives InteractionEvents (mouse, key, etc.) and dispatches
   * them to registered DataInteractor objects. The order in which DataInteractors
   * are offered an event is determined by the "layer" property of their associated
   * DataNode -- higher layers are preferred.
   *
   * After all DataInteractors have been offered the event, registered
   * InteractionEventObserver instances are notified.
   *
   * The Dispatcher supports several processing modes (REGULAR, GRABINPUT,
   * PREFERINPUT, CONNECTEDMOUSEACTION) that control how events are routed
   * during ongoing interactions such as mouse drags.
   *
   * \sa DataInteractor
   * \sa InteractionEventObserver
   * \sa BaseRenderer
   * \ingroup Interaction
   */

  class MITKCORE_EXPORT Dispatcher : public itk::LightObject
  {
  public:
    mitkClassMacroItkParent(Dispatcher, itk::LightObject);
    mitkNewMacro1Param(Self, const std::string &);

    /** \brief List type for storing weak pointers to DataInteractors. */
    typedef std::list<mitk::WeakPointer<DataInteractor>> ListInteractorType;
    /** \brief List type for storing queued InteractionEvents. */
    typedef std::list<itk::SmartPointer<InteractionEvent>> ListEventsType;

    /**
     * \brief Process an interaction event by dispatching it to registered DataInteractors.
     *
     * The event is offered to DataInteractors in order of descending layer.
     * Internal events are handled separately. After dispatching, all registered
     * InteractionEventObservers are notified. Queued events are processed
     * after the current event is fully handled.
     *
     * \param[in] event The interaction event to process.
     * \return true if the event was handled by a DataInteractor, false otherwise.
     */
    bool ProcessEvent(InteractionEvent *event);

    /**
     * \brief Queue an event for deferred processing.
     *
     * Queued events are processed after the currently dispatched event has been
     * fully handled (including state transitions). This allows DataInteractors
     * to post follow-up events without interrupting the current dispatch.
     *
     * \param[in] event The event to enqueue.
     * \note Events queued from outside a DataInteractor/Observer will be delayed
     *       until the next regular event arrives.
     * \note Be careful not to cause infinite loops.
     */
    void QueueEvent(InteractionEvent *event);

    /**
     * \brief Register the DataInteractor associated with the given DataNode.
     *
     * If a DataInteractor for the same DataNode already exists, it is removed first.
     * Orphaned interactors (without valid DataNodes) are also cleaned up.
     *
     * \param[in] dataNode The DataNode whose DataInteractor should be registered.
     */
    void AddDataInteractor(const DataNode *dataNode);

    /**
     * \brief Remove all DataInteractors associated with the given DataNode.
     * \param[in] dataNode The DataNode whose interactors should be removed.
     */
    void RemoveDataInteractor(const DataNode *dataNode);

    /**
     * \brief Get the number of currently registered DataInteractors.
     * \return The number of interactors.
     */
    size_t GetNumberOfInteractors();

  protected:
    Dispatcher(const std::string &rendererName);
    ~Dispatcher() override;

  private:
    ListInteractorType m_Interactors;
    ListEventsType m_QueuedEvents;

    /**
     * Removes all Interactors without a DataNode pointing to them, this is necessary especially when a DataNode is
     * assigned to a new Interactor
     */
    void RemoveOrphanedInteractors();

    /**
     * See \ref DataInteractionTechnicalPage_DispatcherEventDistSection for a description of ProcessEventModes
     */
    ProcessEventMode m_ProcessingMode;
    mitk::WeakPointer<DataInteractor> m_SelectedInteractor;

    void SetEventProcessingMode(DataInteractor *);

    /**
     * Function to handle special internal events,
     * such as events that are directed at a specific DataInteractor,
     * or the request to delete an Interactor and its DataNode.
     */
    bool HandleInternalEvent(InternalEvent *internalEvent);

    /**
     * Hold microservice reference to object that takes care of informing the InteractionEventObservers about
     * InteractionEvents
     */
    us::ServiceTracker<InteractionEventObserver> *m_EventObserverTracker;
  };

} /* namespace mitk */
#endif
