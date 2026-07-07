/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkGizmoInteractor_h
#define mitkGizmoInteractor_h

#include <mitkGizmo.h>

// MITK includes
#include <mitkDataInteractor.h>
#include <mitkDataNode.h>
#include <mitkGeometry3D.h>

// VTK includes
#include <vtkCellPicker.h>
#include <vtkSmartPointer.h>

// System includes
#include <memory>

#include <MitkGizmoExports.h>

namespace mitk
{
  class InteractionPositionEvent;

  /**
   * \brief Data interactor for manipulating object geometry via a Gizmo widget.
   *
   * Unlike other interactors, this class operates on two DataNodes:
   * - The "gizmo node" provides the visual feedback and serves as the
   *   picking target for user input.
   * - The "manipulated object node" owns the geometry that is actually
   *   modified by the user interaction.
   *
   * During interaction, the interactor queries the appropriate mapper
   * (2D: GizmoMapper2D, 3D: standard surface mapper) for VTK props,
   * performs picking, and delegates handle identification to the Gizmo.
   * Based on the picked handle, the interactor applies translation,
   * rotation, or uniform scaling operations to the manipulated object's
   * geometry.
   *
   * All geometry modifications are recorded on the undo/redo stack,
   * enabling full undo support.
   *
   * The interactor's state machine is defined in Gizmo3DStates.xml
   * and configured via Gizmo3DConfig.xml.
   *
   * \sa Gizmo
   * \sa DataInteractor
   */
  class MITKGIZMO_EXPORT GizmoInteractor : public DataInteractor
  {
  public:
    mitkClassMacro(GizmoInteractor, DataInteractor);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

      /**
       * \brief Set the DataNode that holds the Gizmo for visual feedback.
       *
       * This node is used as the interactor's primary data node
       * (calls DataInteractor::SetDataNode internally) and provides the
       * picking target for user interaction.
       *
       * \param[in] node The DataNode containing a Gizmo as its data object.
       *
       * \sa SetManipulatedObjectNode
       */
      void SetGizmoNode(DataNode *node);

    /**
     * \brief Set the DataNode whose geometry shall be manipulated.
     *
     * The geometry of this node's data object will be transformed
     * (translated, rotated, or scaled) in response to user interaction
     * with the gizmo.
     *
     * \param[in] node The DataNode whose geometry is to be manipulated.
     *                  Must hold a data object with a valid geometry.
     *
     * \sa SetGizmoNode
     */
    void SetManipulatedObjectNode(DataNode *node);

  private:
    GizmoInteractor();
    ~GizmoInteractor() override;

    //! Setup the relation between the XML state machine and this object's methods.
    void ConnectActionsAndFunctions() override;

    //! State machine condition: successful gizmo picking
    //! \return true when any part of the gizmo has been picked.
    bool HasPickedHandle(const InteractionEvent *);

    //! State machine action: Advances state machine by generating a new event
    //! in function of the picked part of the gizmo.
    void DecideInteraction(StateMachineAction *, InteractionEvent *interactionEvent);

    //! State machine action: Translates the manipulated object along the picked axis
    //! in function of mouse movement.
    void MoveAlongAxis(StateMachineAction *, InteractionEvent *interactionEvent);

    //! State machine action: Scale the manipulated object equally along all axes
    //! in function of mouse movement.
    void ScaleEqually(StateMachineAction *, InteractionEvent *interactionEvent);

    //! State machine action: Rotates the manipulated object around the picked axis
    //! in function of mouse movement.
    void RotateAroundAxis(StateMachineAction *, InteractionEvent *interactionEvent);

    //! State machine action: Moves the manipulated object parallel to the camera view plane
    //! in function of mouse movement.
    void MoveFreely(StateMachineAction *, InteractionEvent *interactionEvent);

    //! Add the final transformation operations to the undo stack
    void FeedUndoStack(StateMachineAction *, InteractionEvent *interactionEvent);

    //! Applies a calculated translation vector to the manipulated object.
    void ApplyTranslationToManipulatedObject(const Vector3D &projectedMovement);

    //! Applies a calculated scaling factor the manipulated object.
    void ApplyEqualScalingToManipulatedObject(double scalingFactor);

    //! Applies a calculated rotation angle to the manipulated object.
    void ApplyRotationToManipulatedObject(double angle_deg);

    //! Pick a gizmo handle from a 2D event (passing by the 2D mapper)
    Gizmo::HandleType PickFrom2D(const InteractionPositionEvent *positionEvent);

    //! Pick a gizmo handle from a 3D event
    //! (passing by the general surface mapper and the gizmo object)
    Gizmo::HandleType PickFrom3D(const InteractionPositionEvent *positionEvent);

    void UpdateHandleHighlight();

    //! the Gizmo used for visual feedback and picking
    Gizmo::Pointer m_Gizmo;

    //! The manipulated object's geometry
    BaseGeometry::Pointer m_ManipulatedObjectGeometry;

    //! For picking on the vtkPolyData representing the gizmo
    std::map<BaseRenderer *, vtkSmartPointer<vtkCellPicker>> m_Picker;

    //! Part of the gizmo that was picked on last check
    Gizmo::HandleType m_PickedHandle = Gizmo::NoHandle;

    //! Part of the gizmo that is currently highlighted
    Gizmo::HandleType m_HighlightedHandle = Gizmo::NoHandle;

    //! Color (RGBA) used for highlighting
    double m_ColorForHighlight[4];

    //! Color (RGBA) that has been replaced by m_ColorForHighlight
    double m_ColorReplacedByHighlight[4];

    Point2D m_InitialClickPosition2D; //< Initial screen click position
    double m_InitialClickPosition2DZ; //< Z value of the initial screen click position
    Point3D m_InitialClickPosition3D; //< Initial 3D click position

    Point2D m_InitialGizmoCenter2D; //< Initial position of the gizmo's center in screen coordinates
    Point3D m_InitialGizmoCenter3D; //< Initial 3D position of the gizmo's center

    Vector3D m_AxisOfMovement; //< Axis along which we move when translating
    Vector3D m_AxisOfRotation; //< Axis around which we turn when rotating

    std::unique_ptr<Operation> m_FinalDoOperation;   //< Operation for the undo-stack
    std::unique_ptr<Operation> m_FinalUndoOperation; //< Operation for the undo-stack

    //! A copy of the origin geometry, to avoid accumulation of tiny errors
    BaseGeometry::Pointer m_InitialManipulatedObjectGeometry;
  };
}
#endif
