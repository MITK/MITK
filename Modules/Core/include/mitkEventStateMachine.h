/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkEventStateMachine_h
#define mitkEventStateMachine_h

#include <itkObject.h>
#include <mitkCommon.h>
#include <mitkInteractionEventHandler.h>
#include <mitkMessage.h>

#include <MitkCoreExports.h>
#include <string>

/**
 * Macro that can be used to connect a StateMachineAction with a function.
 * It assumes that there is a typedef Classname Self in classes that use
 * this macro, as is provided by e.g. mitkClassMacro
 */
#define CONNECT_FUNCTION(a, f)                                                                                         \
  ::mitk::EventStateMachine::AddActionFunction(                                                                        \
    a,                                                                                                                 \
    ::mitk::MessageDelegate2<Self, ::mitk::StateMachineAction *, ::mitk::InteractionEvent *, void>(this, &Self::f));

#define CONNECT_CONDITION(a, f)                                                                                        \
  ::mitk::EventStateMachine::AddConditionFunction(                                                                     \
    a, ::mitk::MessageDelegate1<Self, const ::mitk::InteractionEvent *, bool>(this, &Self::f));

namespace us
{
  class Module;
}

namespace mitk
{
  class StateMachineTransition;
  class StateMachineContainer;
  class StateMachineAction;
  class StateMachineCondition;
  class InteractionEvent;
  class StateMachineState;
  class DataNode;
  class UndoController;

  /**
   * \class TActionFunctor
   * \brief Base class of ActionFunctors, to provide an easy to connect actions with functions.
   *
   * \deprecatedSince{2013_03} Use mitk::Message classes instead.
   */
  class TActionFunctor
  {
  public:
    virtual bool DoAction(StateMachineAction *, InteractionEvent *) = 0;
    virtual ~TActionFunctor() {}
  };

  ///**
  // * \class TSpecificActionFunctor
  // * Specific implementation of ActionFunctor class, implements a reference to the function which is to be executed.
  // It
  // takes two arguments:
  // * StateMachineAction - the action by which the function call is invoked, InteractionEvent - the event that caused
  // the
  // transition.
  // */
  // template<class T>
  // class DEPRECATED() TSpecificActionFunctor : public TActionFunctor
  //{
  // public:

  //  TSpecificActionFunctor(T* object, bool (T::*memberFunctionPointer)(StateMachineAction*, InteractionEvent*)) :
  //      m_Object(object), m_MemberFunctionPointer(memberFunctionPointer)
  //  {
  //  }

  //  virtual ~TSpecificActionFunctor()
  //  {
  //  }
  //  virtual bool DoAction(StateMachineAction* action, InteractionEvent* event) override
  //  {
  //    return (*m_Object.*m_MemberFunctionPointer)(action, event);// executes member function
  //  }

  // private:
  //  T* m_Object;
  //  bool (T::*m_MemberFunctionPointer)(StateMachineAction*, InteractionEvent*);
  //};

  /**
   * \class EventStateMachine
   *
   * \brief Provides state machine functionality for DataInteractors.
   *
   * A state machine is created by loading an XML state machine pattern via
   * LoadStateMachine(). It consists of states, transitions, and actions:
   * - States represent the current status of the interaction.
   * - Transitions are the means to switch between states; each is triggered by an event.
   * - Actions are executed when a state change is performed.
   *
   * Subclasses override ConnectActionsAndFunctions() to bind action names from the
   * XML description to C++ member functions using the CONNECT_FUNCTION macro.
   * Conditions can be bound similarly using the CONNECT_CONDITION macro.
   *
   * \sa DataInteractor
   * \sa InteractionEventHandler
   * \sa Dispatcher
   * \ingroup Interaction
   */
  class MITKCORE_EXPORT EventStateMachine : public mitk::InteractionEventHandler
  {
  public:
    mitkClassMacro(EventStateMachine, InteractionEventHandler);

      typedef std::map<std::string, TActionFunctor *> DEPRECATED(ActionFunctionsMapType);

    typedef itk::SmartPointer<StateMachineState> StateMachineStateType;

    /**
     * \brief Load a state machine pattern from an XML resource file.
     *
     * The file must be located in the Resources/Interaction folder of the
     * given module. Default is the Mitk module (core). After loading,
     * ConnectActionsAndFunctions() is called to bind actions.
     *
     * \param[in] filename The resource filename (e.g. "PointSetInteraction.xml").
     * \param[in] module The module containing the resource. Defaults to the Mitk module.
     * \return true if the state machine was loaded successfully, false otherwise.
     */
    bool LoadStateMachine(const std::string &filename, const us::Module *module = nullptr);

    /**
     * \brief Process an incoming interaction event.
     *
     * Receives an event from the Dispatcher, maps it to a variant via the
     * EventConfig, checks if the current state has a matching transition, and
     * if so, transitions to the next state and executes all associated actions.
     *
     * \param[in] event The interaction event to handle.
     * \param[in] dataNode The DataNode associated with this interactor.
     * \return true if the event was handled (a transition was executed), false otherwise.
     *
     * \note If a transition has multiple actions, true is returned as soon as one
     *       action succeeds. All actions within a transition should have consistent conditions.
     */
    bool HandleEvent(InteractionEvent *event, DataNode *dataNode);

    /**
     * \brief Enable or disable the undo mechanism for this state machine.
     * \param[in] enable True to enable undo, false to disable.
     */
    void EnableUndo(bool enable) { m_UndoEnabled = enable; }

    /**
     * \brief Enable or disable the state machine.
     *
     * When disabled, the state machine will not react to any events.
     *
     * \param[in] enable True to enable interaction, false to disable.
     */
    void EnableInteraction(bool enable) { m_IsActive = enable; }
  protected:
    EventStateMachine();
    ~EventStateMachine() override;

    typedef MessageAbstractDelegate2<StateMachineAction *, InteractionEvent *, void> ActionFunctionDelegate;
    typedef MessageAbstractDelegate1<const InteractionEvent *, bool> ConditionFunctionDelegate;

    /**
     * Connects action from StateMachine (String in XML file) with a function that is called when this action is to be
     * executed.
     */
    DEPRECATED(void AddActionFunction(const std::string &action, TActionFunctor *functor));

    void AddActionFunction(const std::string &action, const ActionFunctionDelegate &delegate);

    void AddConditionFunction(const std::string &condition, const ConditionFunctionDelegate &delegate);

    StateMachineState *GetCurrentState() const;

    /**
     * @brief ResetToStartState Reset state machine to it initial starting state.
     */

    void ResetToStartState();

    /**
     * Is called after loading a statemachine.
     * Overwrite this function in specific interactor implementations.
     * Connect actions and functions using the  CONNECT_FUNCTION macro within this function.
     */

    virtual void ConnectActionsAndFunctions();

    virtual bool CheckCondition(const StateMachineCondition &condition, const InteractionEvent *interactionEvent);

    /**
     * Looks up function that is associated with action and executes it.
     * To implement your own execution scheme overwrite this in your DataInteractor.
     */
    virtual void ExecuteAction(StateMachineAction *action, InteractionEvent *interactionEvent);

    /**
     * Implements filter scheme for events.
     * Standard implementation accepts events from 2d and 3d windows,
     * and rejects events if DataNode is not visible.
     * \return true if event is accepted, else false
     *
     * Overwrite this function to adapt for your own needs, for example to filter out events from
     * 3d windows like this:
     \code
      bool mitk::EventStateMachine::FilterEvents(InteractionEvent* interactionEvent, DataNode*dataNode)
      {
        return interactionEvent->GetSender()->GetMapperID() == BaseRenderer::Standard2D; // only 2D mappers
      }
     \endcode
     * or to enforce that the interactor only reacts when the corresponding DataNode is selected in the DataManager
     view..
     */
    virtual bool FilterEvents(InteractionEvent *interactionEvent, DataNode *dataNode);

    /** \brief Sets the specified mouse cursor.
     *
     * Use this in subclasses instead of using QmitkApplicationCursor directly.
     */
    void SetMouseCursor(const char *xpm[], int hotspotX, int hotspotY);

    /** \brief Resets the mouse cursor to its original state.
     *
     * Should be used by subclasses and from external application instead
     * of using QmitkApplicationCursor directly to avoid conflicts. */
    void ResetMouseCursor();

    /**
    * \brief Returns the executable transition for the given event.
    *
    * This method takes a list of transitions that correspond to the given
    * event from the current state.
    *
    * This method iterates through all transitions and checks all
    * corresponding conditions. The results of each condition in stored in
    * map, as other transitions may need the same condition again.
    *
    * As soon as a transition is found for which all conditions are
    * fulfilled, this instance is returned.
    *
    * If a transition has no condition, it is automatically returned.
    * If no executable transition is found, nullptr is returned.
    */
    StateMachineTransition *GetExecutableTransition(InteractionEvent *event);

    // Determines if state machine reacts to events
    bool m_IsActive;
    // Undo/Redo
    UndoController *m_UndoController;
    bool m_UndoEnabled;

  private:
    typedef std::map<std::string, ActionFunctionDelegate *> ActionDelegatesMapType;
    typedef std::map<std::string, ConditionFunctionDelegate *> ConditionDelegatesMapType;

    StateMachineContainer
      *m_StateMachineContainer; // storage of all states, action, transitions on which the statemachine operates.
    std::map<std::string, TActionFunctor *> m_ActionFunctionsMap; // stores association between action string
    ActionDelegatesMapType m_ActionDelegatesMap;
    ConditionDelegatesMapType m_ConditionDelegatesMap;
    StateMachineStateType m_CurrentState;

    bool m_MouseCursorSet;
  };

} /* namespace mitk */
#endif
