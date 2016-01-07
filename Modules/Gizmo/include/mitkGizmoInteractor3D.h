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
#include <vtkPointPicker.h>

#include "MitkGizmoExports.h"

namespace mitk
{

class MITKGIZMO_EXPORT GizmoInteractor3D: public DataInteractor
{
public:

  mitkClassMacro(GizmoInteractor3D, DataInteractor);
  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)

  void SetGizmoNode(DataNode* node);
  void SetManipulatedObjectNode(DataNode* node);

protected:

  GizmoInteractor3D();
  virtual ~GizmoInteractor3D();

  virtual void ConnectActionsAndFunctions() override;

  bool HasPickedHandle(const InteractionEvent*);
  void DecideInteraction(StateMachineAction*, InteractionEvent* interactionEvent);
  void MoveAlongAxis(StateMachineAction*, InteractionEvent* interactionEvent);
  void RotateAroundAxis(StateMachineAction*, InteractionEvent* interactionEvent);
  void MoveFreely(StateMachineAction*, InteractionEvent* interactionEvent);
  void ApplyTranslationToManipulatedObject(const Vector3D& projectedMovement);
  void ApplyRotationToManipulatedObject(double angle);
private:

    Gizmo::Pointer m_Gizmo;
    DataNode::Pointer m_ManipulatedObject;

    vtkSmartPointer<vtkPointPicker> m_PointPicker;

    Gizmo::HandleType m_PickedHandle;
    Gizmo::AxisType m_RelevantAxis;

    Point2D m_InitialClickPosition2D;
    double m_InitialClickPosition2DZ;
    Point3D m_InitialClickPosition3D;
    Point2D m_InitialGizmoCenter2D;
    Point3D m_InitialGizmoCenter3D;
    double m_InitialGizmoCenter2DZ;
    Vector3D m_AxisOfMovement;
    Vector3D m_AxisOfRotation;
    BaseGeometry::Pointer m_InitialManipulatedObjectGeometry;
};

}
#endif
