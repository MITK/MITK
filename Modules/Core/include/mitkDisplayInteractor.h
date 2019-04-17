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

#ifndef mitkDisplayInteractor_h
#define mitkDisplayInteractor_h

#include "mitkInteractionEventObserver.h"
#include <MitkCoreExports.h>

namespace mitk
{
  /**
   *\class DisplayInteractor
   *@brief Observer that manages the interaction with the display.
   *
   * This includes the interaction of Zooming, Panning, Scrolling and adjusting the LevelWindow.
   *
   * @ingroup Interaction
   **/
  /**
   * Inherits from mitk::InteractionEventObserver since it doesn't alter any data (only their representation),
   * and its actions cannot be associated with a DataNode. Also inherits from EventStateMachine
   */
  class MITKCORE_EXPORT DisplayInteractor : public EventStateMachine, public InteractionEventObserver
  {
  public:

    mitkClassMacro(DisplayInteractor, EventStateMachine)
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)
    /**
    * By this function the Observer gets notified about new events.
    * Here it is adapted to pass the events to the state machine in order to use
    * its infrastructure.
    * It also checks if event is to be accepted when it already has been processed by a DataInteractor.
    */
    virtual void Notify(InteractionEvent *interactionEvent, bool isHandled) override;

  protected:

    DisplayInteractor();
    ~DisplayInteractor() override;
    /**
     * Derived function.
     * Connects the action names used in the state machine pattern with functions implemented within
     * this InteractionEventObserver. This is only necessary here because the events are processed by the state machine.
     */
    void ConnectActionsAndFunctions() override;
    /**
     * Derived function.
     * Is executed when config object is set / changed.
     * Here it is used to read out the parameters set in the configuration file,
     * and set the member variables accordingly.
     */
    void ConfigurationChanged() override;

    /**
     * Derived function.
     * Is executed when config object is set / changed.
     * Here it is used to read out the parameters set in the configuration file,
     * and set the member variables accordingly.
     */
    bool FilterEvents(InteractionEvent *interactionEvent, DataNode *dataNode) override;

    virtual bool CheckPositionEvent(const InteractionEvent *interactionEvent);

    virtual bool CheckRotationPossible(const InteractionEvent *interactionEvent);

    virtual bool CheckSwivelPossible(const InteractionEvent *interactionEvent);

    /**
     * \brief Initializes an interaction, saves the pointers start position for further reference.
     */
    virtual void Init(StateMachineAction *, InteractionEvent *);
    /**
     * \brief Performs panning of the data set in the render window.
     */
    virtual void Move(StateMachineAction *, InteractionEvent *);

    /**
     * \brief Sets crosshair at clicked position*
     */
    virtual void SetCrosshair(StateMachineAction *, InteractionEvent *);

    /**
     * \brief Increases the time step in 3d+t data
     */
    virtual void IncreaseTimeStep(StateMachineAction *, InteractionEvent *);

    /**
     * \brief Decreases the time step in 3d+t data
     */
    virtual void DecreaseTimeStep(StateMachineAction *, InteractionEvent *);

    /**
     * \brief Performs zooming relative to mouse/pointer movement.
     *
     * Behavior is determined by \see m_ZoomDirection and \see m_ZoomFactor.
     *
     */
    virtual void Zoom(StateMachineAction *, InteractionEvent *);
    /**
     * \brief Performs scrolling relative to mouse/pointer movement.
     *
     * Behavior is determined by \see m_ScrollDirection and \see m_AutoRepeat.
     *
     */
    virtual void Scroll(StateMachineAction *, InteractionEvent *);
    /**
     * \brief Scrolls one layer up
     */
    virtual void ScrollOneDown(StateMachineAction *, InteractionEvent *);
    /**
     * \brief Scrolls one layer down
     */
    virtual void ScrollOneUp(StateMachineAction *, InteractionEvent *);
    /**
     * \brief Adjusts the level windows relative to mouse/pointer movement.
     */
    virtual void AdjustLevelWindow(StateMachineAction *, InteractionEvent *);

    /**
     * \brief Starts crosshair rotation
     */
    virtual void StartRotation(StateMachineAction *, InteractionEvent *);

    /**
     * \brief Ends crosshair rotation
     */
    virtual void EndRotation(StateMachineAction *, InteractionEvent *);

    /**
     * \brief
     */
    virtual void Rotate(StateMachineAction *, InteractionEvent *event);

    virtual void Swivel(StateMachineAction *, InteractionEvent *event);

    /**
     * \brief Updates the Statusbar information with the information about the clicked position
     */
    virtual void UpdateStatusbar(StateMachineAction *, InteractionEvent *event);

    /**
    * \brief Method to retrieve bool-value for given property from string-property
    * in given propertylist.
    */
    bool GetBoolProperty(mitk::PropertyList::Pointer propertyList, const char *propertyName, bool defaultValue);

  private:
    /**
     * @brief UpdateStatusBar
     * @param image3D
     * @param idx
     * @param time
     * @param component If the PixelType of image3D is a vector (for example a 2D velocity vector), then only one of the vector components can be
     * displayed at once. Setting this parameter will determine which of the vector's components will be used to determine the displayed PixelValue.
     * Set this to 0 for scalar images
     */
    void UpdateStatusBar(itk::SmartPointer<mitk::Image> image3D, itk::Index<3> idx, TimeStepType time=0, int component=0);


    /**
     * \brief Coordinate of the pointer at begin of an interaction translated to mm unit
     */
    mitk::Point2D m_StartCoordinateInMM;
    /**
     * \brief Coordinate of the pointer in the last step within an interaction.
     */
    mitk::Point2D m_LastDisplayCoordinate;
    /**
     * \brief Coordinate of the pointer in the last step within an interaction translated to mm unit
     */
    mitk::Point2D m_LastCoordinateInMM;
    /**
     * \brief Current coordinates of the pointer.
     */
    mitk::Point2D m_CurrentDisplayCoordinate;


    /**
     * \brief Modifier that defines how many slices are scrolled per pixel that the mouse has moved
     *
     * This modifier defines how many slices the scene is scrolled per pixel that the mouse cursor has moved.
     * By default the modifier is 4. This means that when the user moves the cursor by 4 pixels in Y-direction
     * the scene is scrolled by one slice. If the user has moved the the cursor by 20 pixels, the scene is
     * scrolled by 5 slices.
     *
     * If the cursor has moved less than m_IndexToSliceModifier pixels the scene is scrolled by one slice.
     */
    int m_IndexToSliceModifier;

    /** Defines behavior at end of data set.
     *  If set to true it will restart at end of data set from the beginning.
     */
    bool m_AutoRepeat;

    /**
    * Defines scroll behavior.
    * Default is up/down movement of pointer performs scrolling
    */
    std::string m_ScrollDirection;

    /**
    * Defines how the axis of interaction influences scroll behavior.
    */
    bool m_InvertScrollDirection;

    /**
    * Defines scroll behavior.
    * Default is up/down movement of pointer performs zooming
    */
    std::string m_ZoomDirection;

    /**
    * Defines how the axis of interaction influences zoom behavior.
    */
    bool m_InvertZoomDirection;

    /**
    * Defines how the axis of interaction influences move behavior.
    */
    bool m_InvertMoveDirection;

    /**
    * Defines level/window behavior.
    * Default is left/right movement of pointer modifies the level.
    */
    std::string m_LevelDirection;

    /**
    * Defines how the axis of interaction influences level/window behavior.
    */
    bool m_InvertLevelWindowDirection;

    /**
     * Determines if the Observer reacts to events that already have been processed by a DataInteractor.
     * The default value is false.
     */
    bool m_AlwaysReact;
    /**
     * Factor to adjust zooming speed.
     */
    float m_ZoomFactor;

    ///// Members to deal with rotating slices

    /**
     * @brief m_LinkPlanes Determines if angle between crosshair remains fixed when rotating
     */
    bool m_LinkPlanes;

    typedef std::vector<SliceNavigationController*> SNCVector;
    SNCVector m_RotatableSNCs; /// all SNCs that currently have CreatedWorldGeometries, that can be rotated.
    SNCVector m_SNCsToBeRotated; /// all SNCs that will be rotated (exceptions are the ones parallel to the one being clicked)

    Point3D m_LastCursorPosition; /// used for calculation of the rotation angle
    Point3D m_CenterOfRotation;   /// used for calculation of the rotation angle

    Point2D m_ReferenceCursor;

    Vector3D m_RotationPlaneNormal;
    Vector3D m_RotationPlaneXVector;
    Vector3D m_RotationPlaneYVector;

    Vector3D m_PreviousRotationAxis;
    ScalarType m_PreviousRotationAngle;
  };
}
#endif
