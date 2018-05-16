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

#ifndef mitkDispatcher_h
#define mitkDispatcher_h

#include "itkLightObject.h"
#include "itkObjectFactory.h"
#include "mitkCommon.h"
#include "mitkDataInteractor.h"
#include "mitkDataNode.h"
#include "usServiceTracker.h"
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
  * \brief Manages event distribution
  *
  * Receives Events (Mouse-,Key-, ... Events) and dispatches them to the registered DataInteractor Objects.
  * The order in which DataInteractors are offered to handle an event is determined by layer of their associated
  * DataNode.
  * Higher layers are preferred.
  *
  * \ingroup Interaction
  */

  class MITKCORE_EXPORT Dispatcher : public itk::LightObject
  {
  public:
    mitkClassMacroItkParent(Dispatcher, itk::LightObject);
    mitkNewMacro1Param(Self, const std::string &);

    typedef std::list<mitk::WeakPointer<DataInteractor>> ListInteractorType;
    typedef std::list<itk::SmartPointer<InteractionEvent>> ListEventsType;

    /**
     * To post new Events which are to be handled by the Dispatcher.
     *
     * @return Returns true if the event has been handled by an DataInteractor, and false else.
     */
    bool ProcessEvent(InteractionEvent *event);

    /**
     * Adds an Event to the Dispatchers EventQueue, these events will be processed after a a regular posted event has
     * been
     * fully handled.
     * This allows DataInteractors to post their own events without interrupting regular Dispatching workflow.
     * It is important to note that the queued events will be processed AFTER the state change of a current transition
     * (which queued the events)
     * is performed.
     *
     * \note 1) If an event is added from an other source than an DataInteractor / Observer its execution will be
     * delayed
     * until the next regular event
     * comes in.
     * \note 2) Make sure you're not causing infinite loops!
     */
    void QueueEvent(InteractionEvent *event);

    /**
     * Adds the DataInteractor that is associated with the DataNode to the Dispatcher Queue.
     * If there already exists an DataInteractor that has a reference to the same DataNode, it is removed.
     * Note that within this method also all other DataInteractors are checked and removed if they are no longer active,
     * and were not removed properly.
     */
    void AddDataInteractor(const DataNode *dataNode);
    /**
     * Remove all DataInteractors related to this Node, to prevent double entries and dead references.
     */
    void RemoveDataInteractor(const DataNode *dataNode);
    size_t GetNumberOfInteractors(); // DEBUG TESTING

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
#endif /* mitkDispatcher_h */
