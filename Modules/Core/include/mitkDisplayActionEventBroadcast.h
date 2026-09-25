/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkDisplayActionEventBroadcast_h
#define mitkDisplayActionEventBroadcast_h

#include <mitkInteractionEventObserver.h>
#include <MitkCoreExports.h>

namespace mitk
{
  /**
   * \brief Event state machine that observes interaction events and broadcasts display action events.
   *
   * Connects the actions from the event state machine XML file with concrete
   * functions of this class. The observed interaction events are mouse events
   * that trigger certain actions according to an event configuration (e.g. PACS
   * mode). These actions typically perform preprocessing and then broadcast a
   * specific display action event (e.g. DisplayMoveEvent, DisplayZoomEvent).
   *
   * Any instance that wants to react to the invoked events can call
   * AddObserver() on a specific broadcast instance, providing an
   * itkEventObject and an itkCommand.
   *
   * \ingroup Interaction
   * \sa DisplayActionEventHandler DisplayActionEvents InteractionEventObserver
   */
  class MITKCORE_EXPORT DisplayActionEventBroadcast : public EventStateMachine, public InteractionEventObserver
  {
  public:
    /**
     * \brief Keeps left mouse button presses away from all broadcasts while it is active.
     *
     * Obtained from BlockLeftButton(). A default-constructed block is inactive,
     * moving transfers it, and destruction or Reset() releases it.
     */
    class MITKCORE_EXPORT LeftButtonBlock
    {
    public:
      LeftButtonBlock() = default;
      LeftButtonBlock(LeftButtonBlock&& other) noexcept;
      LeftButtonBlock& operator=(LeftButtonBlock&& other) noexcept;
      ~LeftButtonBlock();

      LeftButtonBlock(const LeftButtonBlock&) = delete;
      LeftButtonBlock& operator=(const LeftButtonBlock&) = delete;

      /** \brief Releases the block. Does nothing if it is inactive. */
      void Reset();

    private:
      friend class DisplayActionEventBroadcast;

      bool m_IsActive = false;
    };

    mitkClassMacro(DisplayActionEventBroadcast, EventStateMachine);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    /**
     * \brief Suppresses display actions triggered by pressing the left mouse button.
     *
     * Meant for tools that react to the left mouse button as interaction event
     * observers. They are notified about the same unhandled events as the
     * broadcasts, so without a block a single click would also, for example,
     * move the crosshair.
     *
     * The block applies to every broadcast, including broadcasts created while
     * it is held, and is independent of their event configuration, so switching
     * the interaction scheme does not lift it. Blocks are counted: presses are
     * suppressed until the last active block is released, regardless of the
     * order in which they are released. All other events, including mouse moves
     * with the left button held, still reach the broadcasts.
     */
    [[nodiscard]] static LeftButtonBlock BlockLeftButton();

    /**
     * \brief Called for every InteractionEvent observed through the InteractionEventObserver interface.
     *
     * The interaction event is passed to the state machine in order to use
     * its infrastructure for action dispatch. Events that were already handled
     * are skipped unless the configuration sets alwaysReact, and left mouse
     * button presses are skipped while blocked (see BlockLeftButton()).
     *
     * \param interactionEvent The event that was observed and triggered this notification.
     * \param isHandled        Flag indicating whether a DataInteractor has already handled the event.
     *
     * \sa InteractionEventObserver
     */
    void Notify(InteractionEvent* interactionEvent, bool isHandled) override;

  protected:

    /** \brief Constructor. Connects actions and registers as a MicroService. */
    DisplayActionEventBroadcast();
    /** \brief Destructor. Unregisters the MicroService. */
    ~DisplayActionEventBroadcast() override;

    /**
     * \brief Connect the action names from the state machine XML with member functions.
     */
    void ConnectActionsAndFunctions() override;

    /**
     * \brief Called when the event configuration is set or changed.
     *
     * Reads parameters from the configuration file and updates member
     * variables accordingly (scroll direction, zoom factor, etc.).
     */
    void ConfigurationChanged() override;

    /**
     * \brief Filter events by checking the sender renderer.
     *
     * \param interactionEvent The event whose sender is checked.
     * \param dataNode         Ignored in this implementation.
     * \return \c true if the sender is valid and uses a 2D renderer, \c false otherwise.
     */
    bool FilterEvents(InteractionEvent* interactionEvent, DataNode* dataNode) override;

    //////////////////////////////////////////////////////////////////////////
    // Functions to react to interaction events (actions)
    //////////////////////////////////////////////////////////////////////////

    /**
     * \brief Check if the given event is an InteractionPositionEvent.
     * \param interactionEvent The event to check.
     * \return \c true if the event can be cast to InteractionPositionEvent.
     */
    bool CheckPositionEvent(const InteractionEvent* interactionEvent);

    /**
     * \brief Check if rotation is possible given the current interaction state.
     * \param interactionEvent The current interaction event.
     * \return \c true if rotation can be performed.
     */
    bool CheckRotationPossible(const InteractionEvent* interactionEvent);

    /** \brief Initialize interaction coordinates at the start of a gesture. */
    void Init(StateMachineAction* stateMachineAction, InteractionEvent* interactionEvent);

    /** \brief Broadcast a DisplayMoveEvent based on mouse movement. */
    void Move(StateMachineAction* stateMachineAction , InteractionEvent* interactionEvent);

    /** \brief Broadcast a DisplaySetCrosshairEvent at the current position. */
    void SetCrosshair(StateMachineAction* stateMachineAction, InteractionEvent* interactionEvent);

    /** \brief Broadcast a DisplayZoomEvent based on mouse movement. */
    void Zoom(StateMachineAction* stateMachineAction, InteractionEvent* interactionEvent);

    /** \brief Broadcast a DisplayScrollEvent based on mouse movement. */
    void Scroll(StateMachineAction* stateMachineAction, InteractionEvent* interactionEvent);

    /** \brief Broadcast a DisplayScrollEvent to scroll one slice up. */
    void ScrollOneUp(StateMachineAction* stateMachineAction, InteractionEvent* interactionEvent);

    /** \brief Broadcast a DisplayScrollEvent to scroll one slice down. */
    void ScrollOneDown(StateMachineAction* stateMachineAction, InteractionEvent* interactionEvent);

    /** \brief Broadcast a DisplaySetLevelWindowEvent based on mouse movement. */
    void AdjustLevelWindow(StateMachineAction* stateMachineAction, InteractionEvent* interactionEvent);

    /** \brief Begin a plane rotation interaction. */
    void StartRotation(StateMachineAction* stateMachineAction, InteractionEvent* interactionEvent);

    /** \brief End a plane rotation interaction. */
    void EndRotation(StateMachineAction* stateMachineAction, InteractionEvent* interactionEvent);

    /** \brief Perform a plane rotation based on mouse movement. */
    void Rotate(StateMachineAction* stateMachineAction, InteractionEvent* interactionEvent);

    /** \brief Increase the current time step by one. */
    void IncreaseTimeStep(StateMachineAction* stateMachineAction, InteractionEvent* interactionEvent);

    /** \brief Decrease the current time step by one. */
    void DecreaseTimeStep(StateMachineAction* stateMachineAction, InteractionEvent* interactionEvent);

  private:

    /**
     * \brief Read a boolean property from the given property list.
     * \return The property value, or \p defaultValue if not found.
     */
    bool GetBoolProperty(PropertyList::Pointer propertyList, const char* propertyName, bool defaultValue);

    /** \brief Service registration for the InteractionEventObserver MicroService. */
    us::ServiceRegistration<InteractionEventObserver> m_ServiceRegistration;

    /** \brief Whether to react to events already handled by a DataInteractor (default: false). */
    bool m_AlwaysReact;

    /** \brief Mouse pointer coordinate at the beginning of an interaction (in mm). */
    Point2D m_StartCoordinateInMM;
    /** \brief Mouse pointer coordinate from the last step within an interaction. */
    Point2D m_LastDisplayCoordinate;
    /** \brief Current coordinates of the mouse pointer. */
    Point2D m_CurrentDisplayCoordinate;

    /** \brief Whether to restart scrolling from the beginning at the end of a data set. */
    bool m_AutoRepeat;

    /**
     * \brief Number of pixels of mouse movement per slice scroll step.
     *
     * By default 4, meaning 4 pixels of cursor movement scrolls one slice.
     * If the cursor moves less than this value, one slice is still scrolled.
     */
    int m_IndexToSliceModifier;

    /** \brief Axis for scroll direction (default: up/down). */
    std::string m_ScrollDirection;
    /** \brief Whether to invert the scroll direction. */
    bool m_InvertScrollDirection;
    /** \brief Axis for zoom direction (default: up/down). */
    std::string m_ZoomDirection;
    /** \brief Whether to invert the zoom direction. */
    bool m_InvertZoomDirection;
    /** \brief Factor to adjust zooming speed. */
    float m_ZoomFactor;
    /** \brief Whether to invert the move direction. */
    bool m_InvertMoveDirection;
    /** \brief Axis for level-window adjustment (default: left/right). */
    std::string m_LevelDirection;
    /** \brief Whether to invert the level-window direction. */
    bool m_InvertLevelWindowDirection;
    /** \brief Whether to keep the angle between crosshair planes fixed during rotation. */
    bool m_LinkPlanes;

    typedef std::vector<SliceNavigationController*> SNCVector;
    SNCVector m_RotatableSNCs;
    SNCVector m_SNCsToBeRotated;

    Point3D m_LastCursorPosition;
    Point3D m_CenterOfRotation;
  };
} // end namespace

#endif
