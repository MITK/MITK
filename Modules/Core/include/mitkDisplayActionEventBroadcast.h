/*===================================================================

 The Medical Imaging Interaction Toolkit (MITK)

 Copyright (c) German Cancer Research Center,
 Division of Medical Image Computing.
 All rights reserved.

 This software is distributed WITHOUT ANY WARRANTY; without
 even the implied warranty of MERCHANTABILITY or FITNESS FOR
 A PARTICULAR PURPOSE.

 See LICENSE.txt or http://www.mitk.org for details.

 ===================================================================*/

#ifndef MITKDISPLAYACTIONEVENTBROADCAST_H
#define MITKDISPLAYACTIONEVENTBROADCAST_H

#include "mitkInteractionEventObserver.h"
#include <MitkCoreExports.h>

namespace mitk
{
  /**
  * @brief This class serves as an event state machine while simultaneously observing interaction events.
  *   It connects the actions from the event state machine .xml-file with concrete functions of this class.
  *
  *   The observed interaction events are mouse events that trigger certain actions, according to an event configuration (e.g. PACS mode).
  *   These actions are defined and connected inside this broadcast class.
  *   They typically contain some preprocessing steps and use the results of the preprocessing to broadcast a specific display event.
  *
  *   Any instance that wants to react on the invoked events can call 'AddObserver' on a specific broadcast instance,
  *   given an itkEventObject and an itkCommand.
  */
  class MITKCORE_EXPORT DisplayActionEventBroadcast : public EventStateMachine, public InteractionEventObserver
  {
  public:
    mitkClassMacro(DisplayActionEventBroadcast, EventStateMachine)
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    /**
    * By this function this observer is notified about about every 'InteractionEvent'.
    * The interaction event is passed to the state machine in order to use its infrastructure.
    * For more information see @see InteractionEventObserver.
    *
    * @par  interactionEvent    The event that was observed and triggered this notification.
    * @par  isHandled           Flag that indicates if a 'DataInteractor' has already handled the event.
    */
    virtual void Notify(InteractionEvent* interactionEvent, bool isHandled) override;

  protected:

    DisplayActionEventBroadcast();
    virtual ~DisplayActionEventBroadcast() override;

    /**
    * @brief Connects the action names used in the state machine pattern with functions implemented within this InteractionEventObserver.
    */
    void ConnectActionsAndFunctions() override;
    /**
    * @brief This function is executed when a config object is set / changed (via 'SetEventConfig' or 'AddEventConfig' in 'InteractionEventObserver').
    *     It is used to read out the parameters set in the configuration file and to set the member variables accordingly.
    */
    virtual void ConfigurationChanged() override;
    /**
    * @brief Filters the event resp. the sender of the event.
    *
    * @par      interactionEvent    The event whose sender has to be checked
    * @par      data node           The data node is ignored in this specific implementation.
    *
    * @return   True, if the sender of the event is a valid sender and the sending renderer is a 2D-renderer. False, if not.
    */
    virtual bool FilterEvents(InteractionEvent* interactionEvent, DataNode* dataNode) override;

    //////////////////////////////////////////////////////////////////////////
    // Functions to react to interaction events (actions)
    //////////////////////////////////////////////////////////////////////////
    /**
    * @brief Check if the given interaction event is actually an 'InteractionPositionEvent'.
    *
    * @par interactionEvent   The interaction event that is checked.
    *
    * @return True, if the given event can be dynamically cast to an 'InteractionPositionEvent'. False, if not.
    */
    bool CheckPositionEvent(const InteractionEvent* interactionEvent);

    bool CheckRotationPossible(const InteractionEvent* interactionEvent);

    bool CheckSwivelPossible(const InteractionEvent* interactionEvent);

    void Init(StateMachineAction* stateMachineAction, InteractionEvent* interactionEvent);

    void Move(StateMachineAction* stateMachineAction , InteractionEvent* interactionEvent);

    void SetCrosshair(StateMachineAction* stateMachineAction, InteractionEvent* interactionEvent);

    void Zoom(StateMachineAction* stateMachineAction, InteractionEvent* interactionEvent);

    void Scroll(StateMachineAction* stateMachineAction, InteractionEvent* interactionEvent);

    void ScrollOneUp(StateMachineAction* stateMachineAction, InteractionEvent* interactionEvent);

    void ScrollOneDown(StateMachineAction* stateMachineAction, InteractionEvent* interactionEvent);

    void AdjustLevelWindow(StateMachineAction* stateMachineAction, InteractionEvent* interactionEvent);

    void StartRotation(StateMachineAction* stateMachineAction, InteractionEvent* interactionEvent);

    void EndRotation(StateMachineAction* stateMachineAction, InteractionEvent* interactionEvent);

    void Rotate(StateMachineAction* stateMachineAction, InteractionEvent* interactionEvent);

    void Swivel(StateMachineAction* stateMachineAction, InteractionEvent* interactionEvent);

  private:

    void UpdateStatusbar(StateMachineAction* stateMachineAction, InteractionEvent* interactionEvent);

    bool GetBoolProperty(PropertyList::Pointer propertyList, const char* propertyName, bool defaultValue);

    /**
    * @brief Reference to the service registration of the observer.
    *   This is needed to unregister the observer on unload.
    */
    us::ServiceRegistration<InteractionEventObserver> m_ServiceRegistration;

    /**
    * @brief Determines if this broadcast class reacts to events that already have been processed by a DataInteractor.
    *     The default value is false.
    */
    bool m_AlwaysReact;

    /**
    * @brief Coordinate of the mouse pointer at beginning of an interaction (translated to mm unit).
    */
    Point2D m_StartCoordinateInMM;
    /**
    * @brief Coordinate of the mouse pointer in the last step within an interaction.
    */
    Point2D m_LastDisplayCoordinate;
    /**
    * @brief Coordinate of the mouse pointer in the last step within an interaction (translated to mm unit).
    */
    Point2D m_LastCoordinateInMM;
    /**
    * \brief Current coordinates of the pointer.
    */
    Point2D m_CurrentDisplayCoordinate;

    /**
    * @brief Defines the behavior at the end of a data set.
    *     If set to true, it will restart at end of data set from the beginning.
    */
    bool m_AutoRepeat;
    /**
    * @brief Defines how many slices are scrolled per pixel that the mouse pointer was moved.
    *     By default the modifier is 4. This means that when the user moves the cursor by 4 pixels in Y-direction,
    *     the scene is scrolled by one slice. If the user has moved the the cursor by 20 pixels, the scene is
    *     scrolled by 5 slices.
    *
    *     If the cursor has moved less than m_IndexToSliceModifier pixels, the scene is scrolled by one slice.
    */
    int m_IndexToSliceModifier;
    /**
    * @brief Defines the scroll behavior.
    *     Default is up/down movement of pointer performs scrolling
    */
    std::string m_ScrollDirection;
    /**
    * @brief Defines how the axis of interaction influences scroll behavior.
    */
    bool m_InvertScrollDirection;
    /**
    * @brief Defines the zoom behavior.
    *     Default is up/down movement of pointer performs zooming
    */
    std::string m_ZoomDirection;
    /**
    * @brief Defines how the axis of interaction influences zoom behavior.
    */
    bool m_InvertZoomDirection;
    /**
    * @brief Factor to adjust zooming speed.
    */
    float m_ZoomFactor;
    /**
    * @brief Defines how the axis of interaction influences move behavior.
    */
    bool m_InvertMoveDirection;
    /**
    * @brief Defines the level-window behavior.
    *     Default is left/right movement of pointer modifies the level.
    */
    std::string m_LevelDirection;
    /**
    * @brief Defines how the axis of interaction influences level-window behavior.
    */
    bool m_InvertLevelWindowDirection;
    /**
    * @brief Determines if the angle between crosshair remains fixed when rotating.
    */
    bool m_LinkPlanes;

    typedef std::vector<SliceNavigationController*> SNCVector;
    SNCVector m_RotatableSNCs;
    SNCVector m_SNCsToBeRotated;

    Point3D m_LastCursorPosition;
    Point3D m_CenterOfRotation;

    Point2D m_ReferenceCursor;

    Vector3D m_RotationPlaneNormal;
    Vector3D m_RotationPlaneXVector;
    Vector3D m_RotationPlaneYVector;

    Vector3D m_PreviousRotationAxis;
    ScalarType m_PreviousRotationAngle;
  };
} // end namespace

#endif // MITKDISPLAYACTIONEVENTBROADCAST_H
