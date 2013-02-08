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

#include "itkObject.h"
#include "itkObjectFactory.h"
#include "mitkCommon.h"
#include "mitkDataNode.h"
#include "mitkDataInteractor.h"
#include <MitkExports.h>
#include <list>


namespace mitk
{

  enum ProcessEventMode
  {
    REGULAR,
    GRABINPUT,
    PREFERINPUT,
    CONNECTEDMOUSEACTION
  };

  class InternalEvent;
  class InteractionEvent;
  class InformerService;

  /**
  * \class Dispatcher
  * \brief Manages event distribution
  *
  * Receives Events (Mouse-,Key-, ... Events) and dispatches them to the registered DataInteractor Objects.
  * The order in which DataInteractors are offered to handle an event is determined by layer of their associated DataNode.
  * Higher layers are preferred.
  */

  class MITK_CORE_EXPORT Dispatcher: public itk::Object
  {

  public:
    mitkClassMacro(Dispatcher, itk::Object);
    itkNewMacro(Self);

    typedef std::list<DataInteractor::Pointer> ListInteractorType;
    typedef std::list<itk::SmartPointer<InteractionEvent> > ListEventsType;

    /**
     * To post new Events which are to be handled by the Dispatcher.
     *
     * @return Returns true if the event has been handled by an DataInteractor, and false else.
     */
    bool ProcessEvent(InteractionEvent* event);

    /**
     * Adds an Event to the Dispatchers EventQueue, these events will be processed after a a regular posted event has been fully handled.
     * This allows Interactors to post their own events without interrupting regular Dispatching workflow.
     * NOTES: 1) If an event is added from an other source than an Interactor / Listener its execution will be delayed until the next regular event
     * comes in.
     * 2) Make sure you're not causing infinite loops!
     *
     */

    void QueueEvent(InteractionEvent* event);

    /**
     * Adds the DataInteractor that is associated with the DataNode to the Dispatcher Queue.
     * If there already exists an DataInteractor that has a reference to the same DataNode, it is removed.
     * Note that within this method also all other DataInteractors are checked and removed if they are no longer active,
     * and were not removed properly.
     */
    void AddDataInteractor(const DataNode* dataNode);
    /**
     * Remove all DataInteractors related to this Node, to prevent double entries and dead references.
     */
    void RemoveDataInteractor(const DataNode* dataNode);
    size_t GetNumberOfInteractors(); // DEBUG TESTING

  protected:
    Dispatcher();
    virtual ~Dispatcher();

  private:

    struct cmp{
      bool operator()(DataInteractor::Pointer d1, DataInteractor::Pointer d2){
        return (d1->GetLayer() > d2->GetLayer());
      }
    };
    std::list<DataInteractor::Pointer> m_Interactors;
    ListEventsType m_QueuedEvents;

    /**
     * Removes all Interactors without a DataNode pointing to them, this is necessary especially when a DataNode is assigned to a new Interactor
     */
    void RemoveOrphanedInteractors();

    /**
     * See \ref DispatcherEventDistSection for a description of ProcessEventModes
     */
    ProcessEventMode m_ProcessingMode;
    DataInteractor::Pointer m_SelectedInteractor;

    void SetEventProcessingMode(DataInteractor::Pointer);

    /**
     * Function to handle special internal events,
     * such as events that are directed at a specific DataInteractor,
     * or the request to delete an Interactor and its DataNode.
     */
    bool HandleInternalEvent(InternalEvent* internalEvent);

    /**
     * Hold microservice reference to object that takes care of informing the EventObservers about InteractionEvents
     */
    InformerService* m_InformerService;

  };

} /* namespace mitk */
#endif /* mitkDispatcher_h */
