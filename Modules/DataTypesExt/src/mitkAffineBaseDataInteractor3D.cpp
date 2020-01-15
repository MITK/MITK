/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkAffineBaseDataInteractor3D.h"

#include <mitkInteractionConst.h>
#include <mitkInteractionPositionEvent.h>
#include <mitkRotationOperation.h>
#include <mitkScaleOperation.h>
#include <mitkSurface.h>

#include <mitkInteractionKeyEvent.h>
#include <vtkCamera.h>
#include <vtkInteractorObserver.h>
#include <vtkInteractorStyle.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkRenderWindowInteractor.h>

// Properties to allow the user to interact with the base data
const char *translationStepSizePropertyName = "AffineBaseDataInteractor3D.Translation Step Size";
const char *selectedColorPropertyName = "AffineBaseDataInteractor3D.Selected Color";
const char *deselectedColorPropertyName = "AffineBaseDataInteractor3D.Deselected Color";
const char *priorPropertyName = "AffineBaseDataInteractor3D.Prior Color";
const char *rotationStepSizePropertyName = "AffineBaseDataInteractor3D.Rotation Step Size";
const char *scaleStepSizePropertyName = "AffineBaseDataInteractor3D.Scale Step Size";
const char *anchorPointX = "AffineBaseDataInteractor3D.Anchor Point X";
const char *anchorPointY = "AffineBaseDataInteractor3D.Anchor Point Y";
const char *anchorPointZ = "AffineBaseDataInteractor3D.Anchor Point Z";

mitk::AffineBaseDataInteractor3D::AffineBaseDataInteractor3D()
{
  m_OriginalGeometry = mitk::Geometry3D::New();
}

mitk::AffineBaseDataInteractor3D::~AffineBaseDataInteractor3D()
{
  this->RestoreNodeProperties();
}

void mitk::AffineBaseDataInteractor3D::ConnectActionsAndFunctions()
{
  // **Conditions** that can be used in the state machine, to ensure that certain conditions are met, before actually
  // executing an action
  CONNECT_CONDITION("isOverObject", CheckOverObject);

  // **Function** in the statmachine patterns also referred to as **Actions**
  CONNECT_FUNCTION("selectObject", SelectObject);
  CONNECT_FUNCTION("deselectObject", DeselectObject);
  CONNECT_FUNCTION("initTranslate", InitTranslate);
  CONNECT_FUNCTION("initRotate", InitRotate);
  CONNECT_FUNCTION("translateObject", TranslateObject);
  CONNECT_FUNCTION("rotateObject", RotateObject);
  CONNECT_FUNCTION("scaleObject", ScaleObject);

  CONNECT_FUNCTION("translateUpKey", TranslateUpKey);
  CONNECT_FUNCTION("translateDownKey", TranslateDownKey);
  CONNECT_FUNCTION("translateLeftKey", TranslateLeftKey);
  CONNECT_FUNCTION("translateRightKey", TranslateRightKey);
  CONNECT_FUNCTION("translateUpModifierKey", TranslateUpModifierKey);
  CONNECT_FUNCTION("translateDownModifierKey", TranslateDownModifierKey);

  CONNECT_FUNCTION("scaleDownKey", ScaleDownKey);
  CONNECT_FUNCTION("scaleUpKey", ScaleUpKey);

  CONNECT_FUNCTION("rotateUpKey", RotateUpKey);
  CONNECT_FUNCTION("rotateDownKey", RotateDownKey);
  CONNECT_FUNCTION("rotateLeftKey", RotateLeftKey);
  CONNECT_FUNCTION("rotateRightKey", RotateRightKey);
  CONNECT_FUNCTION("rotateUpModifierKey", RotateUpModifierKey);
  CONNECT_FUNCTION("rotateDownModifierKey", RotateDownModifierKey);
}

void mitk::AffineBaseDataInteractor3D::TranslateUpKey(StateMachineAction *, InteractionEvent *interactionEvent)
{
  float stepSize = 1.0f;
  this->GetDataNode()->GetFloatProperty(translationStepSizePropertyName, stepSize);
  mitk::Vector3D movementVector;
  movementVector.Fill(0.0);
  movementVector.SetElement(2, stepSize);
  this->TranslateGeometry(movementVector, this->GetUpdatedTimeGeometry(interactionEvent));
}

void mitk::AffineBaseDataInteractor3D::TranslateDownKey(mitk::StateMachineAction *,
                                                        mitk::InteractionEvent *interactionEvent)
{
  float stepSize = 1.0f;
  this->GetDataNode()->GetFloatProperty(translationStepSizePropertyName, stepSize);
  mitk::Vector3D movementVector;
  movementVector.Fill(0.0);
  movementVector.SetElement(2, -stepSize);
  this->TranslateGeometry(movementVector, this->GetUpdatedTimeGeometry(interactionEvent));
}

void mitk::AffineBaseDataInteractor3D::TranslateLeftKey(mitk::StateMachineAction *,
                                                        mitk::InteractionEvent *interactionEvent)
{
  float stepSize = 1.0f;
  this->GetDataNode()->GetFloatProperty(translationStepSizePropertyName, stepSize);
  mitk::Vector3D movementVector;
  movementVector.Fill(0.0);
  movementVector.SetElement(0, -stepSize);
  this->TranslateGeometry(movementVector, this->GetUpdatedTimeGeometry(interactionEvent));
}

void mitk::AffineBaseDataInteractor3D::TranslateRightKey(mitk::StateMachineAction *,
                                                         mitk::InteractionEvent *interactionEvent)
{
  float stepSize = 1.0f;
  this->GetDataNode()->GetFloatProperty(translationStepSizePropertyName, stepSize);
  mitk::Vector3D movementVector;
  movementVector.Fill(0.0);
  movementVector.SetElement(0, stepSize);
  this->TranslateGeometry(movementVector, this->GetUpdatedTimeGeometry(interactionEvent));
}

void mitk::AffineBaseDataInteractor3D::TranslateUpModifierKey(mitk::StateMachineAction *,
                                                              mitk::InteractionEvent *interactionEvent)
{
  float stepSize = 1.0f;
  this->GetDataNode()->GetFloatProperty(translationStepSizePropertyName, stepSize);
  mitk::Vector3D movementVector;
  movementVector.Fill(0.0);
  movementVector.SetElement(1, stepSize);
  this->TranslateGeometry(movementVector, this->GetUpdatedTimeGeometry(interactionEvent));
}

void mitk::AffineBaseDataInteractor3D::TranslateDownModifierKey(mitk::StateMachineAction *,
                                                                mitk::InteractionEvent *interactionEvent)
{
  float stepSize = 1.0f;
  this->GetDataNode()->GetFloatProperty(translationStepSizePropertyName, stepSize);
  mitk::Vector3D movementVector;
  movementVector.Fill(0.0);
  movementVector.SetElement(1, -stepSize);
  this->TranslateGeometry(movementVector, this->GetUpdatedTimeGeometry(interactionEvent));
}

void mitk::AffineBaseDataInteractor3D::RotateUpKey(mitk::StateMachineAction *, mitk::InteractionEvent *interactionEvent)
{
  float stepSize = 1.0f;
  this->GetDataNode()->GetFloatProperty(rotationStepSizePropertyName, stepSize);
  this->RotateGeometry(-stepSize, 0, this->GetUpdatedTimeGeometry(interactionEvent));
}

void mitk::AffineBaseDataInteractor3D::RotateDownKey(mitk::StateMachineAction *,
                                                     mitk::InteractionEvent *interactionEvent)
{
  float stepSize = 1.0f;
  this->GetDataNode()->GetFloatProperty(rotationStepSizePropertyName, stepSize);
  this->RotateGeometry(stepSize, 0, this->GetUpdatedTimeGeometry(interactionEvent));
  return;
}

void mitk::AffineBaseDataInteractor3D::RotateLeftKey(mitk::StateMachineAction *,
                                                     mitk::InteractionEvent *interactionEvent)
{
  float stepSize = 1.0f;
  this->GetDataNode()->GetFloatProperty(rotationStepSizePropertyName, stepSize);
  this->RotateGeometry(-stepSize, 2, this->GetUpdatedTimeGeometry(interactionEvent));
}

void mitk::AffineBaseDataInteractor3D::RotateRightKey(mitk::StateMachineAction *,
                                                      mitk::InteractionEvent *interactionEvent)
{
  float stepSize = 1.0f;
  this->GetDataNode()->GetFloatProperty(rotationStepSizePropertyName, stepSize);
  this->RotateGeometry(stepSize, 2, this->GetUpdatedTimeGeometry(interactionEvent));
}

void mitk::AffineBaseDataInteractor3D::RotateUpModifierKey(mitk::StateMachineAction *,
                                                           mitk::InteractionEvent *interactionEvent)
{
  float stepSize = 1.0f;
  this->GetDataNode()->GetFloatProperty(rotationStepSizePropertyName, stepSize);
  this->RotateGeometry(stepSize, 1, this->GetUpdatedTimeGeometry(interactionEvent));
}

void mitk::AffineBaseDataInteractor3D::RotateDownModifierKey(mitk::StateMachineAction *,
                                                             mitk::InteractionEvent *interactionEvent)
{
  float stepSize = 1.0f;
  this->GetDataNode()->GetFloatProperty(rotationStepSizePropertyName, stepSize);
  this->RotateGeometry(-stepSize, 1, this->GetUpdatedTimeGeometry(interactionEvent));
}

void mitk::AffineBaseDataInteractor3D::ScaleUpKey(mitk::StateMachineAction *, mitk::InteractionEvent *interactionEvent)
{
  float stepSize = 0.1f;
  this->GetDataNode()->GetFloatProperty(scaleStepSizePropertyName, stepSize);
  mitk::Point3D newScale;
  newScale.Fill(stepSize);
  this->ScaleGeometry(newScale, this->GetUpdatedTimeGeometry(interactionEvent));
}

void mitk::AffineBaseDataInteractor3D::ScaleDownKey(mitk::StateMachineAction *,
                                                    mitk::InteractionEvent *interactionEvent)
{
  float stepSize = 0.1f;
  this->GetDataNode()->GetFloatProperty(scaleStepSizePropertyName, stepSize);
  mitk::Point3D newScale;
  newScale.Fill(-stepSize);
  this->ScaleGeometry(newScale, this->GetUpdatedTimeGeometry(interactionEvent));
}

void mitk::AffineBaseDataInteractor3D::ScaleGeometry(mitk::Point3D newScale, mitk::BaseGeometry *geometry)
{
  mitk::Point3D anchorPoint;
  float pointX = 0.0f;
  float pointY = 0.0f;
  float pointZ = 0.0f;
  anchorPoint.Fill(0.0);
  this->GetDataNode()->GetFloatProperty(anchorPointX, pointX);
  this->GetDataNode()->GetFloatProperty(anchorPointY, pointY);
  this->GetDataNode()->GetFloatProperty(anchorPointZ, pointZ);
  anchorPoint[0] = pointX;
  anchorPoint[1] = pointY;
  anchorPoint[2] = pointZ;

  auto *doOp = new mitk::ScaleOperation(OpSCALE, newScale, anchorPoint);
  geometry->ExecuteOperation(doOp);

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void mitk::AffineBaseDataInteractor3D::RotateGeometry(mitk::ScalarType angle,
                                                      int rotationaxis,
                                                      mitk::BaseGeometry *geometry)
{
  mitk::Vector3D rotationAxis = geometry->GetAxisVector(rotationaxis);
  float pointX = 0.0f;
  float pointY = 0.0f;
  float pointZ = 0.0f;
  mitk::Point3D pointOfRotation;
  pointOfRotation.Fill(0.0);
  this->GetDataNode()->GetFloatProperty(anchorPointX, pointX);
  this->GetDataNode()->GetFloatProperty(anchorPointY, pointY);
  this->GetDataNode()->GetFloatProperty(anchorPointZ, pointZ);
  pointOfRotation[0] = pointX;
  pointOfRotation[1] = pointY;
  pointOfRotation[2] = pointZ;

  auto *doOp = new mitk::RotationOperation(OpROTATE, pointOfRotation, rotationAxis, angle);

  geometry->ExecuteOperation(doOp);
  delete doOp;
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void mitk::AffineBaseDataInteractor3D::TranslateGeometry(mitk::Vector3D translate, mitk::BaseGeometry *geometry)
{
  geometry->Translate(translate);
  this->GetDataNode()->Modified();
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

mitk::BaseGeometry *mitk::AffineBaseDataInteractor3D::GetUpdatedTimeGeometry(mitk::InteractionEvent *interactionEvent)
{
  // Get the correct time geometry to support 3D + t
  int timeStep = interactionEvent->GetSender()->GetTimeStep(this->GetDataNode()->GetData());
  BaseGeometry *geometry = this->GetDataNode()->GetData()->GetUpdatedTimeGeometry()->GetGeometryForTimeStep(timeStep);
  if (geometry == nullptr)
    MITK_ERROR << "Geometry is nullptr. Cannot modify it.";
  return geometry;
}

void mitk::AffineBaseDataInteractor3D::DataNodeChanged()
{
  mitk::DataNode::Pointer newInputNode = this->GetDataNode();
  if (newInputNode.IsNotNull())
  {
    // add default properties
    newInputNode->AddProperty(selectedColorPropertyName, mitk::ColorProperty::New(0.0, 1.0, 0.0));
    newInputNode->AddProperty(deselectedColorPropertyName, mitk::ColorProperty::New(0.0, 0.0, 1.0));
    newInputNode->AddProperty(translationStepSizePropertyName, mitk::FloatProperty::New(1.0f));
    newInputNode->AddProperty(rotationStepSizePropertyName, mitk::FloatProperty::New(1.0f));
    newInputNode->AddProperty(scaleStepSizePropertyName, mitk::FloatProperty::New(0.1f));

    // save the previous color of the node, in order to restore it after the interactor is destroyed
    mitk::ColorProperty::Pointer priorColor = dynamic_cast<mitk::ColorProperty *>(newInputNode->GetProperty("color"));
    if (priorColor.IsNotNull())
    {
      mitk::ColorProperty::Pointer tmpCopyOfPriorColor = mitk::ColorProperty::New();
      tmpCopyOfPriorColor->SetColor(priorColor->GetColor());
      newInputNode->AddProperty(priorPropertyName, tmpCopyOfPriorColor);
    }
    newInputNode->SetColor(0.0, 0.0, 1.0);
  }
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void mitk::AffineBaseDataInteractor3D::SetDataNode(DataNode *node)
{
  this->RestoreNodeProperties(); // if there was another node set, restore it's color
  DataInteractor::SetDataNode(node);
}

bool mitk::AffineBaseDataInteractor3D::CheckOverObject(const InteractionEvent *interactionEvent)
{
  const auto *positionEvent = dynamic_cast<const InteractionPositionEvent *>(interactionEvent);
  if (positionEvent == nullptr)
    return false;

  Point3D currentWorldPoint;
  if (interactionEvent->GetSender()->PickObject(positionEvent->GetPointerPositionOnScreen(), currentWorldPoint) ==
      this->GetDataNode())
    return true;

  return false;
}

void mitk::AffineBaseDataInteractor3D::SelectObject(StateMachineAction *, InteractionEvent *)
{
  DataNode::Pointer node = this->GetDataNode();

  if (node.IsNull())
    return;

  mitk::ColorProperty::Pointer selectedColor =
    dynamic_cast<mitk::ColorProperty *>(node->GetProperty(selectedColorPropertyName));
  if (selectedColor.IsNotNull())
  {
    node->GetPropertyList()->SetProperty("color", selectedColor);
  }
  RenderingManager::GetInstance()->RequestUpdateAll();

  return;
}

void mitk::AffineBaseDataInteractor3D::DeselectObject(StateMachineAction *, InteractionEvent *)
{
  DataNode::Pointer node = this->GetDataNode();

  if (node.IsNull())
    return;

  mitk::ColorProperty::Pointer selectedColor =
    dynamic_cast<mitk::ColorProperty *>(node->GetProperty(deselectedColorPropertyName));
  if (selectedColor.IsNotNull())
  {
    node->GetPropertyList()->SetProperty("color", selectedColor);
  }

  RenderingManager::GetInstance()->RequestUpdateAll();

  return;
}

void mitk::AffineBaseDataInteractor3D::InitTranslate(StateMachineAction *, InteractionEvent *interactionEvent)
{
  InitMembers(interactionEvent);
}

void mitk::AffineBaseDataInteractor3D::InitRotate(StateMachineAction *, InteractionEvent *interactionEvent)
{
  InitMembers(interactionEvent);
}

bool mitk::AffineBaseDataInteractor3D::InitMembers(InteractionEvent *interactionEvent)
{
  auto *positionEvent = dynamic_cast<InteractionPositionEvent *>(interactionEvent);
  if (positionEvent == nullptr)
    return false;

  m_InitialPickedDisplayPoint = positionEvent->GetPointerPositionOnScreen();
  m_InitialPickedWorldPoint = positionEvent->GetPositionInWorld();

  // Get the timestep to also support 3D+t
  int timeStep = 0;
  if ((interactionEvent->GetSender()) != nullptr)
    timeStep = interactionEvent->GetSender()->GetTimeStep(this->GetDataNode()->GetData());

  // Make deep copy of current Geometry3D of the plane
  this->GetDataNode()->GetData()->UpdateOutputInformation(); // make sure that the Geometry is up-to-date
  m_OriginalGeometry =
    static_cast<Geometry3D *>(this->GetDataNode()->GetData()->GetGeometry(timeStep)->Clone().GetPointer());
  return true;
}

void mitk::AffineBaseDataInteractor3D::TranslateObject(StateMachineAction *, InteractionEvent *interactionEvent)
{
  auto *positionEvent = dynamic_cast<InteractionPositionEvent *>(interactionEvent);
  if (positionEvent == nullptr)
    return;

  Point3D currentPickedPoint = positionEvent->GetPositionInWorld();

  Vector3D interactionMove;
  interactionMove[0] = currentPickedPoint[0] - m_InitialPickedWorldPoint[0];
  interactionMove[1] = currentPickedPoint[1] - m_InitialPickedWorldPoint[1];
  interactionMove[2] = currentPickedPoint[2] - m_InitialPickedWorldPoint[2];

  int timeStep = interactionEvent->GetSender()->GetTimeStep(this->GetDataNode()->GetData());

  mitk::BaseGeometry::Pointer geometry =
    this->GetDataNode()->GetData()->GetUpdatedTimeGeometry()->GetGeometryForTimeStep(timeStep);
  geometry->SetOrigin(m_OriginalGeometry->GetOrigin());

  this->TranslateGeometry(interactionMove, this->GetUpdatedTimeGeometry(interactionEvent));

  return;
}

void mitk::AffineBaseDataInteractor3D::RotateObject(StateMachineAction *, InteractionEvent *interactionEvent)
{
  auto *positionEvent = dynamic_cast<InteractionPositionEvent *>(interactionEvent);
  if (positionEvent == nullptr)
    return;

  Point2D currentPickedDisplayPoint = positionEvent->GetPointerPositionOnScreen();
  Point3D currentWorldPoint = positionEvent->GetPositionInWorld();

  vtkCamera *camera = nullptr;
  vtkRenderer *currentVtkRenderer = nullptr;

  if ((interactionEvent->GetSender()) != nullptr)
  {
    camera = interactionEvent->GetSender()->GetVtkRenderer()->GetActiveCamera();
    currentVtkRenderer = interactionEvent->GetSender()->GetVtkRenderer();
  }
  if (camera && currentVtkRenderer)
  {
    double vpn[3];
    camera->GetViewPlaneNormal(vpn);

    Vector3D viewPlaneNormal;
    viewPlaneNormal[0] = vpn[0];
    viewPlaneNormal[1] = vpn[1];
    viewPlaneNormal[2] = vpn[2];

    Vector3D interactionMove;
    interactionMove[0] = currentWorldPoint[0] - m_InitialPickedWorldPoint[0];
    interactionMove[1] = currentWorldPoint[1] - m_InitialPickedWorldPoint[1];
    interactionMove[2] = currentWorldPoint[2] - m_InitialPickedWorldPoint[2];

    if (interactionMove[0] == 0 && interactionMove[1] == 0 && interactionMove[2] == 0)
      return;

    Vector3D rotationAxis = itk::CrossProduct(viewPlaneNormal, interactionMove);
    rotationAxis.Normalize();

    int *size = currentVtkRenderer->GetSize();
    double l2 = (currentPickedDisplayPoint[0] - m_InitialPickedDisplayPoint[0]) *
                  (currentPickedDisplayPoint[0] - m_InitialPickedDisplayPoint[0]) +
                (currentPickedDisplayPoint[1] - m_InitialPickedDisplayPoint[1]) *
                  (currentPickedDisplayPoint[1] - m_InitialPickedDisplayPoint[1]);

    double rotationAngle = 360.0 * sqrt(l2 / (size[0] * size[0] + size[1] * size[1]));

    // Use center of data bounding box as center of rotation
    Point3D rotationCenter = m_OriginalGeometry->GetCenter();

    int timeStep = 0;
    if ((interactionEvent->GetSender()) != nullptr)
      timeStep = interactionEvent->GetSender()->GetTimeStep(this->GetDataNode()->GetData());

    // Reset current Geometry3D to original state (pre-interaction) and
    // apply rotation
    RotationOperation op(OpROTATE, rotationCenter, rotationAxis, rotationAngle);
    Geometry3D::Pointer newGeometry = static_cast<Geometry3D *>(m_OriginalGeometry->Clone().GetPointer());
    newGeometry->ExecuteOperation(&op);
    mitk::TimeGeometry::Pointer timeGeometry = this->GetDataNode()->GetData()->GetTimeGeometry();
    if (timeGeometry.IsNotNull())
      timeGeometry->SetTimeStepGeometry(newGeometry, timeStep);

    RenderingManager::GetInstance()->RequestUpdateAll();
  }
}

void mitk::AffineBaseDataInteractor3D::ScaleObject(StateMachineAction *, InteractionEvent * /*interactionEvent*/)
{
  return;
}

void mitk::AffineBaseDataInteractor3D::RestoreNodeProperties()
{
  mitk::DataNode::Pointer inputNode = this->GetDataNode();
  if (inputNode.IsNull())
    return;
  mitk::ColorProperty::Pointer color = dynamic_cast<mitk::ColorProperty *>(inputNode->GetProperty(priorPropertyName));
  if (color.IsNotNull())
  {
    inputNode->GetPropertyList()->SetProperty("color", color);
  }

  inputNode->GetPropertyList()->DeleteProperty(selectedColorPropertyName);
  inputNode->GetPropertyList()->DeleteProperty(deselectedColorPropertyName);
  inputNode->GetPropertyList()->DeleteProperty(priorPropertyName);
  inputNode->GetPropertyList()->DeleteProperty(translationStepSizePropertyName);
  inputNode->GetPropertyList()->DeleteProperty(rotationStepSizePropertyName);
  inputNode->GetPropertyList()->DeleteProperty(scaleStepSizePropertyName);

  // update rendering
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}
