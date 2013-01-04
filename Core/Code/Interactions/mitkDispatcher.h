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

/**
 * Receives Events (Mouse-,Key-, ... Events) and dispatches them to the registered DataInteractor Objects.
 * The order in which DataInteractors are offered to handle an event is determined by layer of their associated DataNode.
 * Higher layers are preferred.
 */

namespace mitk
{

  enum ProcessEventMode
  {
    REGULAR, GRABINPUT, PREFERINPUT, CONNECTEDMOUSEACTION
  };

  class InteractionEvent;
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
        return (d1->GetLayer() < d2->GetLayer());
      }
    };
    std::list<DataInteractor::Pointer> m_Interactors;
    ListEventsType m_QueuedEvents;

    /**
     * Removes ALL Interactors without a DataNode pointing to them, this is necessary especially when a DataNode is assigned to a new Interactor
     */
    void RemoveOrphanedInteractors();

    /**
     * The Dispatcher operates in different processing modes, which determine the way the interactor that receives an event is chosen.
     * Those modes are:
     *
     * - Regular
     *
     * - Connected Mouse Action:
     * A connected mouse action is described by the sequence of Mouse-Press, [Mouse-Move] , Mouse-Release Events.
     * Within this sequence all events are sent to the same Interactor, namely the one which received the event from the Mouse-Press action.
     * m_ConnectedMouseAction  - is set to true, when a Mouse-Down Event occurs and an interactor takes the events,
     * m_SelectedInteractor contains said interactor.
     * m_ConnectedMouseAction is reset to false, after the Mouse-Release Event occurs,
     * while it is true, the m_SelectedInteractor is the only one that receives Mouse-Events.
     *
     * - Grab Input
     * Whenever an interactor performs a state change into a state that is marked by the grab input-tag, the dispatcher switches into this mode.
     * As long as it is in this mode ONLY the selected interactor will receive the event. This mode is ended when the interactor switches back to
     * a state without a tag/ or the REGULAR-tag.
     * NOTE: Does NOT inform Listeners.
     *
     * - Prefer Input
     * Basically works as Grab Input, with the difference that if the Interactor in an prefer input state, cannot process the event offered,
     * it will be offered to the other interactors in the regular way.
     * Does inform Listeners.
     */
    ProcessEventMode m_ProcessingMode;
    DataInteractor::Pointer m_SelectedInteractor;

    void SetEventProcessingMode(DataInteractor::Pointer);


  };

} /* namespace mitk */
#endif /* mitkDispatcher_h */
