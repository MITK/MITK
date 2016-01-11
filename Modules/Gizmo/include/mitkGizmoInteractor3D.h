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

#ifndef mitkGizmoInteractor3D_h
#define mitkGizmoInteractor3D_h

#include "mitkGizmo.h"

// MITK includes
#include <mitkDataInteractor.h>
#include <mitkGeometry3D.h>
#include <mitkDataNode.h>

// VTK includes
#include <vtkSmartPointer.h>
#include <vtkCellPicker.h>

#include "MitkGizmoExports.h"

namespace mitk
{

//! Data interactor to manipulate the geometry of an object via interaction
//! with a mitk::Gizmo visualization.
//!
//! Unlike other interactors, this class needs two DataNodes to work on:
//! - the "manipulated object" is the one whose geometry is modified by user input
//! - the "gizmo node" represents the manipulated object's primary axes _and_ is
//!   used to take user input (most importantly: to have a picking target)
//!
//! \sa Gizmo
class MITKGIZMO_EXPORT GizmoInteractor3D: public DataInteractor
{
public:

  mitkClassMacro(GizmoInteractor3D, DataInteractor);
  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)

  //! The node holding the gizmo for visual feedback.
  //! This is the node that the interactor is primarily working on
  //! (calls DataInteractor::SetDataNode).
  void SetGizmoNode(DataNode* node);

  //! The node that shall be manipulated in function of the user
  //! interaction on the gizmo.
  void SetManipulatedObjectNode(DataNode* node);

private:

  GizmoInteractor3D();
  virtual ~GizmoInteractor3D();

  //! Setup the relation between the XML state machine and this object's methods.
  virtual void ConnectActionsAndFunctions() override;

  //! State machine condition: successful gizmo picking
  //! \return true when any part of the gizmo has been picked.
  bool HasPickedHandle(const InteractionEvent*);

  //! State machine action: Advances state machine by generating a new event
  //! in function of the picked part of the gizmo.
  void DecideInteraction(StateMachineAction*, InteractionEvent* interactionEvent);

  //! State machine action: Translates the manipulated object along the picked axis
  //! in function of mouse movement.
  void MoveAlongAxis(StateMachineAction*, InteractionEvent* interactionEvent);

  //! State machine action: Scale the manipulated object equally along all axes
  //! in function of mouse movement.
  void ScaleEqually(StateMachineAction*, InteractionEvent* interactionEvent);

  //! State machine action: Rotates the manipulated object around the picked axis
  //! in function of mouse movement.
  void RotateAroundAxis(StateMachineAction*, InteractionEvent* interactionEvent);

  //! State machine action: Moves the manipulated object parallel to the camera view plane
  //! in function of mouse movement.
  void MoveFreely(StateMachineAction*, InteractionEvent* interactionEvent);

  //! Applies a calculated translation vector to the manipulated object.
  void ApplyTranslationToManipulatedObject(const Vector3D& projectedMovement);

  //! Applies a calculated scaling factor the manipulated object.
  void ApplyEqualScalingToManipulatedObject(double scalingFactor);

  //! Applies a calculated rotation angle to the manipulated object.
  void ApplyRotationToManipulatedObject(double angle_deg);
  //! the Gizmo used for visual feedback and picking
  Gizmo::Pointer m_Gizmo;

  //! The manipulated object's geometry
  BaseGeometry::Pointer m_ManipulatedObjectGeometry;

  //! For picking on the vtkPolyData representing the gizmo
  vtkSmartPointer<vtkCellPicker> m_Picker;

  //! Part of the gizmo that was clicked initially
  Gizmo::HandleType m_PickedHandle;

  Point2D m_InitialClickPosition2D; //< Initial screen click position
  double m_InitialClickPosition2DZ; //< Z value of the initial screen click position
  Point3D m_InitialClickPosition3D; //< Initial 3D click position

  Point2D m_InitialGizmoCenter2D; //< Initial position of the gizmo's center in screen coordinates
  Point3D m_InitialGizmoCenter3D; //< Initial 3D position of the gizmo's center

  Vector3D m_AxisOfMovement; //< Axis along which we move when translating
  Vector3D m_AxisOfRotation; //< Axis around which we turn when rotating

  //! A copy of the origin geometry, to avoid accumulation of tiny errors
  BaseGeometry::Pointer m_InitialManipulatedObjectGeometry;
};

}
#endif
