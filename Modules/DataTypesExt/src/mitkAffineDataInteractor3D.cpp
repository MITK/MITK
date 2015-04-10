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

#include "mitkAffineDataInteractor3D.h"

#include <mitkInteractionConst.h>
#include <mitkInteractionPositionEvent.h>
#include <mitkRotationOperation.h>
#include <mitkSurface.h>

#include <vtkCamera.h>
#include <vtkInteractorStyle.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkInteractorObserver.h>
#include <mitkInteractionKeyEvent.h>

const char* translationStepSizePropertyName = "AffineDataInteractor3D.Translation Step Size";
const char* selectedColorPropertyName       = "AffineDataInteractor3D.Selected Color";
const char* deselectedColorPropertyName     = "AffineDataInteractor3D.Deselected Color";
const char* priorPropertyName               = "AffineDataInteractor3D.Prior Color";
const char* rotationStepSizePropertyName    = "AffineDataInteractor3D.Rotation Step Size";
const char* scaleStepSizePropertyName       = "AffineDataInteractor3D.Scale Step Size";

mitk::AffineDataInteractor3D::AffineDataInteractor3D()
{
  m_OriginalGeometry = mitk::Geometry3D::New();

  // Initialize vector arithmetic
  m_ObjectNormal[0] = 0.0;
  m_ObjectNormal[1] = 0.0;
  m_ObjectNormal[2] = 1.0;
}

mitk::AffineDataInteractor3D::~AffineDataInteractor3D()
{
  this->RestoreNodeProperties();
}

void mitk::AffineDataInteractor3D::ConnectActionsAndFunctions()
{
  // **Conditions** that can be used in the state machine, to ensure that certain conditions are met, before actually executing an action
  CONNECT_CONDITION("isOverObject", CheckOverObject);

  // **Function** in the statmachine patterns also referred to as **Actions**
  CONNECT_FUNCTION("selectObject",SelectObject);
  CONNECT_FUNCTION("deselectObject",DeselectObject);
  CONNECT_FUNCTION("initTranslate",InitTranslate);
  CONNECT_FUNCTION("initRotate",InitRotate);
  CONNECT_FUNCTION("translateObject",TranslateObject);
  CONNECT_FUNCTION("rotateObject",RotateObject);

  CONNECT_FUNCTION("translateUpKey",TranslateUpKey);
  CONNECT_FUNCTION("translateDownKey",TranslateDownKey);
  CONNECT_FUNCTION("translateLeftKey",TranslateLeftKey);
  CONNECT_FUNCTION("translateRightKey",TranslateRightKey);
  CONNECT_FUNCTION("translateUpModifierKey",TranslateUpModifierKey);
  CONNECT_FUNCTION("translateDownModifierKey",TranslateDownModifierKey);

  CONNECT_FUNCTION("scaleDownKey",ScaleDownKey);
  CONNECT_FUNCTION("scaleUpKey",ScaleUpKey);

  CONNECT_FUNCTION("rotateUpKey",RotateUpKey);
  CONNECT_FUNCTION("rotateDownKey",RotateDownKey);
  CONNECT_FUNCTION("rotateLeftKey",RotateLeftKey);
  CONNECT_FUNCTION("rotateRightKey",RotateRightKey);
  CONNECT_FUNCTION("rotateUpModifierKey",RotateUpModifierKey);
  CONNECT_FUNCTION("rotateDownModifierKey",RotateDownModifierKey);
}

bool mitk::AffineDataInteractor3D::TranslateUpKey(StateMachineAction*, InteractionEvent* interactionEvent)
{
  float stepSize = 1.0f;
  this->GetDataNode()->GetFloatProperty(translationStepSizePropertyName, stepSize);
  mitk::Vector3D movementVector;
  movementVector.Fill(0.0);
  movementVector.SetElement(2, stepSize);
  this->TranslateGeometry(movementVector, this->GetUpdatedTimeGeometry(interactionEvent));
  return true;
}

bool mitk::AffineDataInteractor3D::TranslateDownKey(mitk::StateMachineAction*, mitk::InteractionEvent* interactionEvent)
{
  float stepSize = 1.0f;
  this->GetDataNode()->GetFloatProperty(translationStepSizePropertyName, stepSize);
  mitk::Vector3D movementVector;
  movementVector.Fill(0.0);
  movementVector.SetElement(2, -stepSize);
  this->TranslateGeometry(movementVector, this->GetUpdatedTimeGeometry(interactionEvent));
  return true;
}

bool mitk::AffineDataInteractor3D::TranslateLeftKey(mitk::StateMachineAction*, mitk::InteractionEvent* interactionEvent)
{
  float stepSize = 1.0f;
  this->GetDataNode()->GetFloatProperty(translationStepSizePropertyName, stepSize);
  mitk::Vector3D movementVector;
  movementVector.Fill(0.0);
  movementVector.SetElement(0, -stepSize);
  this->TranslateGeometry(movementVector, this->GetUpdatedTimeGeometry(interactionEvent));
  return true;
}

bool mitk::AffineDataInteractor3D::TranslateRightKey(mitk::StateMachineAction*, mitk::InteractionEvent* interactionEvent)
{
  float stepSize = 1.0f;
  this->GetDataNode()->GetFloatProperty(translationStepSizePropertyName, stepSize);
  mitk::Vector3D movementVector;
  movementVector.Fill(0.0);
  movementVector.SetElement(0, stepSize);
  this->TranslateGeometry(movementVector, this->GetUpdatedTimeGeometry(interactionEvent));
  return true;
}

bool mitk::AffineDataInteractor3D::TranslateUpModifierKey(mitk::StateMachineAction*, mitk::InteractionEvent* interactionEvent)
{
  float stepSize = 1.0f;
  this->GetDataNode()->GetFloatProperty(translationStepSizePropertyName, stepSize);
  mitk::Vector3D movementVector;
  movementVector.Fill(0.0);
  movementVector.SetElement(1, stepSize);
  this->TranslateGeometry(movementVector, this->GetUpdatedTimeGeometry(interactionEvent));
  return true;
}

bool mitk::AffineDataInteractor3D::TranslateDownModifierKey(mitk::StateMachineAction*, mitk::InteractionEvent* interactionEvent)
{
  float stepSize = 1.0f;
  this->GetDataNode()->GetFloatProperty(translationStepSizePropertyName, stepSize);
  mitk::Vector3D movementVector;
  movementVector.Fill(0.0);
  movementVector.SetElement(1, -stepSize);
  this->TranslateGeometry(movementVector, this->GetUpdatedTimeGeometry(interactionEvent));
  return true;
}

bool mitk::AffineDataInteractor3D::RotateUpKey(mitk::StateMachineAction*, mitk::InteractionEvent* interactionEvent)
{
  float stepSize = 1.0f;
  this->GetDataNode()->GetFloatProperty(rotationStepSizePropertyName, stepSize);
  this->RotateGeometry(-stepSize, 0, this->GetUpdatedTimeGeometry(interactionEvent));
  return true;
}

bool mitk::AffineDataInteractor3D::RotateDownKey(mitk::StateMachineAction*, mitk::InteractionEvent* interactionEvent)
{
  float stepSize = 1.0f;
  this->GetDataNode()->GetFloatProperty(rotationStepSizePropertyName, stepSize);
  this->RotateGeometry(stepSize, 0, this->GetUpdatedTimeGeometry(interactionEvent));
  return true;
}

bool mitk::AffineDataInteractor3D::RotateLeftKey(mitk::StateMachineAction*, mitk::InteractionEvent* interactionEvent)
{
  float stepSize = 1.0f;
  this->GetDataNode()->GetFloatProperty(rotationStepSizePropertyName, stepSize);
  this->RotateGeometry(-stepSize, 2, this->GetUpdatedTimeGeometry(interactionEvent));
  return true;
}

bool mitk::AffineDataInteractor3D::RotateRightKey(mitk::StateMachineAction*, mitk::InteractionEvent* interactionEvent)
{
  float stepSize = 1.0f;
  this->GetDataNode()->GetFloatProperty(rotationStepSizePropertyName, stepSize);
  this->RotateGeometry(stepSize, 2, this->GetUpdatedTimeGeometry(interactionEvent));
  return true;
}

bool mitk::AffineDataInteractor3D::RotateUpModifierKey(mitk::StateMachineAction*, mitk::InteractionEvent* interactionEvent)
{
  float stepSize = 1.0f;
  this->GetDataNode()->GetFloatProperty(rotationStepSizePropertyName, stepSize);
  this->RotateGeometry(stepSize, 1, this->GetUpdatedTimeGeometry(interactionEvent));
  return true;
}

bool mitk::AffineDataInteractor3D::RotateDownModifierKey(mitk::StateMachineAction*, mitk::InteractionEvent* interactionEvent)
{
  float stepSize = 1.0f;
  this->GetDataNode()->GetFloatProperty(rotationStepSizePropertyName, stepSize);
  this->RotateGeometry(-stepSize, 1, this->GetUpdatedTimeGeometry(interactionEvent));
  return true;
}

bool mitk::AffineDataInteractor3D::ScaleUpKey(mitk::StateMachineAction*, mitk::InteractionEvent* interactionEvent)
{
  float stepSize = 0.1f;
  this->GetDataNode()->GetFloatProperty(scaleStepSizePropertyName, stepSize);
  mitk::Point3D newScale;
  newScale.Fill(stepSize);
  this->ScaleGeometry(newScale, this->GetUpdatedTimeGeometry(interactionEvent));
  return true;
}

bool mitk::AffineDataInteractor3D::ScaleDownKey(mitk::StateMachineAction*, mitk::InteractionEvent* interactionEvent)
{
  float stepSize = 0.1f;
  this->GetDataNode()->GetFloatProperty(scaleStepSizePropertyName, stepSize);
  mitk::Point3D newScale;
  newScale.Fill(-stepSize);
  this->ScaleGeometry(newScale, this->GetUpdatedTimeGeometry(interactionEvent));
  return true;
}

void mitk::AffineDataInteractor3D::ScaleGeometry(mitk::Point3D newScale, mitk::BaseGeometry* geometry)
{
  PointOperation* doOp = new mitk::PointOperation(OpSCALE, newScale, 0);
  geometry->ExecuteOperation(doOp);

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void mitk::AffineDataInteractor3D::RotateGeometry(mitk::ScalarType angle, int rotationaxis, mitk::BaseGeometry* geometry)
{
  mitk::Vector3D rotationAxis = geometry->GetAxisVector(rotationaxis);
  mitk::Point3D center = geometry->GetCenter();

  mitk::RotationOperation* doOp = new mitk::RotationOperation(OpROTATE, center, rotationAxis, angle);

  geometry->ExecuteOperation(doOp);
  delete doOp;
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void mitk::AffineDataInteractor3D::TranslateGeometry(mitk::Vector3D translate, mitk::BaseGeometry* geometry)
{
  geometry->Translate(translate);
  this->GetDataNode()->Modified();
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

mitk::BaseGeometry* mitk::AffineDataInteractor3D::GetUpdatedTimeGeometry(mitk::InteractionEvent* interactionEvent)
{
  //Get the correct time geometry to support 3D + t
  int timeStep = interactionEvent->GetSender()->GetTimeStep(this->GetDataNode()->GetData());
  BaseGeometry* geometry = this->GetDataNode()->GetData()->GetUpdatedTimeGeometry()->GetGeometryForTimeStep( timeStep );
  if(geometry == NULL)
    MITK_ERROR << "Geometry is NULL. Cannot modify it.";
  return geometry;
}

void mitk::AffineDataInteractor3D::DataNodeChanged()
{
  mitk::DataNode::Pointer newInputNode = this->GetDataNode();
  //add default properties
  newInputNode->AddProperty( selectedColorPropertyName, mitk::ColorProperty::New(0.0,1.0,0.0) );
  newInputNode->AddProperty( deselectedColorPropertyName, mitk::ColorProperty::New(0.0,0.0,1.0) );
  newInputNode->AddProperty( translationStepSizePropertyName, mitk::FloatProperty::New(1.0f) );
  newInputNode->AddProperty( rotationStepSizePropertyName, mitk::FloatProperty::New(1.0f) );
  newInputNode->AddProperty( scaleStepSizePropertyName, mitk::FloatProperty::New(0.1f) );

  //save the previous color of the node, in order to restore it after the interactor is destroyed
  mitk::ColorProperty::Pointer priorColor = dynamic_cast<mitk::ColorProperty*>(newInputNode->GetProperty("color"));
  if ( priorColor.IsNotNull() )
  {
      mitk::ColorProperty::Pointer tmpCopyOfPriorColor = mitk::ColorProperty::New();
      tmpCopyOfPriorColor->SetColor( priorColor->GetColor() );
      newInputNode->AddProperty( priorPropertyName, tmpCopyOfPriorColor );
  }
  newInputNode->SetColor(0.0,0.0,1.0);
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void mitk::AffineDataInteractor3D::SetDataNode(mitk::DataInteractor::NodeType node)
{
  this->RestoreNodeProperties(); //if there was another node set, restore it's color
  DataInteractor::SetDataNode(node);
}

bool mitk::AffineDataInteractor3D::CheckOverObject(const InteractionEvent* interactionEvent)
{
  const InteractionPositionEvent* positionEvent = dynamic_cast<const InteractionPositionEvent*>(interactionEvent);
  if(positionEvent == NULL)
    return false;

  Point3D currentWorldPoint;
  if(interactionEvent->GetSender()->PickObject(positionEvent->GetPointerPositionOnScreen(), currentWorldPoint) == this->GetDataNode().GetPointer())
    return true;

  return false;
}

bool mitk::AffineDataInteractor3D::SelectObject(StateMachineAction*, InteractionEvent* interactionEvent)
{
  DataNode::Pointer node = this->GetDataNode();

  if (node.IsNull())
    return false;

  mitk::ColorProperty::Pointer selectedColor = dynamic_cast<mitk::ColorProperty*>(node->GetProperty(selectedColorPropertyName));
  if ( selectedColor.IsNotNull() )
  {
    node->GetPropertyList()->SetProperty("color", selectedColor);
  }
  interactionEvent->GetSender()->GetRenderingManager()->RequestUpdateAll();

  return true;
}

bool mitk::AffineDataInteractor3D::DeselectObject(StateMachineAction*, InteractionEvent* interactionEvent)
{
  DataNode::Pointer node = this->GetDataNode();

  if (node.IsNull())
    return false;

  mitk::ColorProperty::Pointer selectedColor = dynamic_cast<mitk::ColorProperty*>(node->GetProperty(deselectedColorPropertyName));
  if ( selectedColor.IsNotNull() )
  {
    node->GetPropertyList()->SetProperty("color", selectedColor);
  }

  interactionEvent->GetSender()->GetRenderingManager()->RequestUpdateAll();

  return true;
}

bool mitk::AffineDataInteractor3D::InitTranslate(StateMachineAction*, InteractionEvent* interactionEvent)
{
  InteractionPositionEvent* positionEvent = dynamic_cast<InteractionPositionEvent*>(interactionEvent);
  if(positionEvent == NULL)
    return false;

  m_InitialPickedDisplayPoint = positionEvent->GetPointerPositionOnScreen();
  m_InitialPickedWorldPoint = positionEvent->GetPositionInWorld();


  // Get the timestep to also support 3D+t
  int timeStep = 0;
  if ((interactionEvent->GetSender()) != NULL)
    timeStep = interactionEvent->GetSender()->GetTimeStep(this->GetDataNode()->GetData());

  // Make deep copy of current Geometry3D of the plane
  this->GetDataNode()->GetData()->UpdateOutputInformation(); // make sure that the Geometry is up-to-date
  m_OriginalGeometry = static_cast<Geometry3D*>(this->GetDataNode()->GetData()->GetGeometry(timeStep)->Clone().GetPointer());

  return true;
}

bool mitk::AffineDataInteractor3D::InitRotate(StateMachineAction*, InteractionEvent* interactionEvent)
{
  InteractionPositionEvent* positionEvent = dynamic_cast<InteractionPositionEvent*>(interactionEvent);

  m_InitialPickedDisplayPoint = positionEvent->GetPointerPositionOnScreen();
  m_InitialPickedWorldPoint = positionEvent->GetPositionInWorld();

  // Get the timestep to also support 3D+t
  int timeStep = interactionEvent->GetSender()->GetTimeStep(this->GetDataNode()->GetData());

  // Make deep copy of current Geometry3D of the plane
  this->GetDataNode()->GetData()->UpdateOutputInformation(); // make sure that the Geometry is up-to-date
  m_OriginalGeometry = static_cast<Geometry3D*>(this->GetDataNode()->GetData()->GetGeometry(timeStep)->Clone().GetPointer());

  return true;
}

bool mitk::AffineDataInteractor3D::TranslateObject (StateMachineAction*, InteractionEvent* interactionEvent)
{
  InteractionPositionEvent* positionEvent = dynamic_cast<InteractionPositionEvent*>(interactionEvent);
  if(positionEvent == NULL)
    return false;

  double currentWorldPoint[4];
  mitk::Point2D currentDisplayPoint = positionEvent->GetPointerPositionOnScreen();
  vtkInteractorObserver::ComputeDisplayToWorld(
    interactionEvent->GetSender()->GetVtkRenderer(),
    currentDisplayPoint[0],
    currentDisplayPoint[1],
    0.0, //m_InitialInteractionPickedPoint[2],
    currentWorldPoint);

  Vector3D interactionMove;
  interactionMove[0] = currentWorldPoint[0] - m_InitialPickedWorldPoint[0];
  interactionMove[1] = currentWorldPoint[1] - m_InitialPickedWorldPoint[1];
  interactionMove[2] = currentWorldPoint[2] - m_InitialPickedWorldPoint[2];

  Point3D origin = m_OriginalGeometry->GetOrigin();

  // Get the timestep to also support 3D+t
  int timeStep = interactionEvent->GetSender()->GetTimeStep(this->GetDataNode()->GetData());

  // If data is an mitk::Surface, extract it
  Surface::Pointer surface = dynamic_cast< Surface* >(this->GetDataNode()->GetData());
  vtkPolyData* polyData = NULL;
  if (surface.IsNotNull())
  {
    polyData = surface->GetVtkPolyData( timeStep );

    // Extract surface normal from surface (if existent, otherwise use default)
    vtkPointData* pointData = polyData->GetPointData();
    if (pointData != NULL)
    {
      vtkDataArray* normal = polyData->GetPointData()->GetVectors("planeNormal");
      if (normal != NULL)
      {
        m_ObjectNormal[0] = normal->GetComponent( 0, 0 );
        m_ObjectNormal[1] = normal->GetComponent( 0, 1 );
        m_ObjectNormal[2] = normal->GetComponent( 0, 2 );
      }
    }
  }

  Vector3D transformedObjectNormal;
  this->GetDataNode()->GetData()->GetGeometry( timeStep )->IndexToWorld(m_ObjectNormal, transformedObjectNormal);

  this->GetDataNode()->GetData()->GetGeometry( timeStep )->SetOrigin(origin + transformedObjectNormal * (interactionMove * transformedObjectNormal));

  interactionEvent->GetSender()->GetRenderingManager()->RequestUpdateAll();

  return true;
}

bool mitk::AffineDataInteractor3D::RotateObject (StateMachineAction*, InteractionEvent* interactionEvent)
{
  InteractionPositionEvent* positionEvent = dynamic_cast<InteractionPositionEvent*>(interactionEvent);
  if(positionEvent == NULL)
    return false;

  double currentWorldPoint[4];
  Point2D currentPickedDisplayPoint = positionEvent->GetPointerPositionOnScreen();
  vtkInteractorObserver::ComputeDisplayToWorld(
    interactionEvent->GetSender()->GetVtkRenderer(),
    currentPickedDisplayPoint[0],
    currentPickedDisplayPoint[1],
    0.0, //m_InitialInteractionPickedPoint[2],
    currentWorldPoint);

  vtkCamera* camera = NULL;
  vtkRenderer* currentVtkRenderer = NULL;

  if ((interactionEvent->GetSender()) != NULL)
  {
    vtkRenderWindow* renderWindow = interactionEvent->GetSender()->GetRenderWindow();
    if (renderWindow != NULL)
    {
      vtkRenderWindowInteractor* renderWindowInteractor = renderWindow->GetInteractor();
      if ( renderWindowInteractor != NULL )
      {
        currentVtkRenderer = renderWindowInteractor->GetInteractorStyle()->GetCurrentRenderer();
        if (currentVtkRenderer != NULL)
          camera = currentVtkRenderer->GetActiveCamera();
      }
    }
  }
  if ( camera )
  {
    double vpn[3];
    camera->GetViewPlaneNormal( vpn );

    Vector3D viewPlaneNormal;
    viewPlaneNormal[0] = vpn[0];
    viewPlaneNormal[1] = vpn[1];
    viewPlaneNormal[2] = vpn[2];

    Vector3D interactionMove;
    interactionMove[0] = currentWorldPoint[0] - m_InitialPickedWorldPoint[0];
    interactionMove[1] = currentWorldPoint[1] - m_InitialPickedWorldPoint[1];
    interactionMove[2] = currentWorldPoint[2] - m_InitialPickedWorldPoint[2];

    if (interactionMove[0] == 0 && interactionMove[1] == 0  && interactionMove[2] == 0)
      return true;

    Vector3D rotationAxis = itk::CrossProduct(viewPlaneNormal, interactionMove);
    rotationAxis.Normalize();

    int* size = currentVtkRenderer->GetSize();
    double l2 =
      (currentPickedDisplayPoint[0] - m_InitialPickedDisplayPoint[0]) *
      (currentPickedDisplayPoint[0] - m_InitialPickedDisplayPoint[0]) +
      (currentPickedDisplayPoint[1] - m_InitialPickedDisplayPoint[1]) *
      (currentPickedDisplayPoint[1] - m_InitialPickedDisplayPoint[1]);

    double rotationAngle = 360.0 * sqrt(l2 / (size[0] * size[0] + size[1] * size[1]));

    // Use center of data bounding box as center of rotation
    Point3D rotationCenter = m_OriginalGeometry->GetCenter();

    int timeStep = 0;
    if ((interactionEvent->GetSender()) != NULL)
      timeStep = interactionEvent->GetSender()->GetTimeStep(this->GetDataNode()->GetData());

    // Reset current Geometry3D to original state (pre-interaction) and
    // apply rotation
    RotationOperation op( OpROTATE, rotationCenter, rotationAxis, rotationAngle );
    Geometry3D::Pointer newGeometry = static_cast<Geometry3D*>(m_OriginalGeometry->Clone().GetPointer());
    newGeometry->ExecuteOperation( &op );
    mitk::TimeGeometry::Pointer timeGeometry = this->GetDataNode()->GetData()->GetTimeGeometry();
    if (timeGeometry.IsNotNull())
      timeGeometry->SetTimeStepGeometry(newGeometry, timeStep);

    interactionEvent->GetSender()->GetRenderingManager()->RequestUpdateAll();

    return true;
  }
  else
    return false;
}

void mitk::AffineDataInteractor3D::RestoreNodeProperties()
{
  mitk::DataNode::Pointer inputNode = this->GetDataNode();
  if(inputNode.IsNull())
    return;
  mitk::ColorProperty::Pointer color = dynamic_cast<mitk::ColorProperty*>(inputNode->GetProperty(priorPropertyName));
  if ( color.IsNotNull() )
  {
    inputNode->GetPropertyList()->SetProperty("color", color);
  }

  inputNode->GetPropertyList()->DeleteProperty(selectedColorPropertyName);
  inputNode->GetPropertyList()->DeleteProperty(deselectedColorPropertyName);
  inputNode->GetPropertyList()->DeleteProperty(priorPropertyName);
  inputNode->GetPropertyList()->DeleteProperty(translationStepSizePropertyName);
  inputNode->GetPropertyList()->DeleteProperty(rotationStepSizePropertyName);
  inputNode->GetPropertyList()->DeleteProperty(scaleStepSizePropertyName);

  //update rendering
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}
