/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "../DataManagement/mitkBoundingShapeUtil.h"
#include <mitkBoundingShapeInteractor.h>
#include <mitkDisplayActionEventBroadcast.h>
#include <mitkInteractionConst.h>
#include <mitkInteractionEventObserver.h>
#include <mitkInteractionKeyEvent.h>
#include <mitkInteractionPositionEvent.h>
#include <mitkMouseWheelEvent.h>

#include <vtkCamera.h>
#include <vtkInteractorObserver.h>
#include <vtkInteractorStyle.h>
#include <vtkPointData.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkSmartPointer.h>

#include "usGetModuleContext.h"
#include "usModuleRegistry.h"

// Properties to allow the user to interact with the base data
const char *selectedColorPropertyName = "Bounding Shape.Selected Color";
const char *deselectedColorPropertyName = "Bounding Shape.Deselected Color";
const char *activeHandleIdPropertyName = "Bounding Shape.Active Handle ID";
const char *boundingShapePropertyName = "Bounding Shape";

namespace mitk
{
  itkEventMacroDefinition(BoundingShapeInteractionEvent, itk::AnyEvent);

  class BoundingShapeInteractor::Impl
  {
  public:
    Impl() : OriginalInteractionEnabled(false), RotationEnabled(false)
    {
      Point3D initialPoint;
      initialPoint.Fill(0.0);

      for (int i = 0; i < 6; ++i)
        Handles.push_back(Handle(initialPoint, i, GetHandleIndices(i)));
    }

    ~Impl() {}
    bool OriginalInteractionEnabled;
    Point3D InitialPickedWorldPoint;
    Point3D LastPickedWorldPoint;
    Point2D InitialPickedDisplayPoint;
    std::vector<Handle> Handles;
    Handle ActiveHandle;
    Geometry3D::Pointer OriginalGeometry;
    bool RotationEnabled;
    std::map<us::ServiceReferenceU, mitk::EventConfig> DisplayInteractionConfigs;
  };
}

mitk::BoundingShapeInteractor::BoundingShapeInteractor() : m_Impl(new Impl)
{
}

mitk::BoundingShapeInteractor::~BoundingShapeInteractor()
{
  this->RestoreNodeProperties();
  delete m_Impl;
}

void mitk::BoundingShapeInteractor::ConnectActionsAndFunctions()
{
  // **Conditions** that can be used in the state machine, to ensure that certain conditions are met, before actually
  // executing an action
  CONNECT_CONDITION("isHoveringOverObject", CheckOverObject);
  CONNECT_CONDITION("isHoveringOverHandles", CheckOverHandles);

  // **Function** in the statemachine patterns also referred to as **Actions**
  CONNECT_FUNCTION("selectObject", SelectObject);
  CONNECT_FUNCTION("deselectObject", DeselectObject);
  CONNECT_FUNCTION("deselectHandles", DeselectHandles);
  CONNECT_FUNCTION("initInteraction", InitInteraction);
  CONNECT_FUNCTION("translateObject", TranslateObject);
  CONNECT_FUNCTION("selectHandle", SelectHandle);
  CONNECT_FUNCTION("scaleObject", ScaleObject);
  // CONNECT_FUNCTION("rotateObject",RotateObject);
}

// RotateObject(StateMachineAction*, InteractionEvent* interactionEvent)
// void mitk::BoundingShapeInteractor::RotateGeometry(mitk::ScalarType angle, int rotationaxis, mitk::BaseGeometry*
// geometry)
//{
//  mitk::Vector3D rotationAxis = geometry->GetAxisVector(rotationaxis);
//  float pointX = 0.0f;
//  float pointY = 0.0f;
//  float pointZ = 0.0f;
//  mitk::Point3D pointOfRotation;
//  pointOfRotation.Fill(0.0);
//  this->GetDataNode()->GetFloatProperty(anchorPointX, pointX);
//  this->GetDataNode()->GetFloatProperty(anchorPointY, pointY);
//  this->GetDataNode()->GetFloatProperty(anchorPointZ, pointZ);
//  pointOfRotation[0] = pointX;
//  pointOfRotation[1] = pointY;
//  pointOfRotation[2] = pointZ;
//
//  mitk::RotationOperation* doOp = new mitk::RotationOperation(OpROTATE, pointOfRotation, rotationAxis, angle);
//
//  geometry->ExecuteOperation(doOp);
//  delete doOp;
//}

void mitk::BoundingShapeInteractor::SetRotationEnabled(bool rotationEnabled)
{
  m_Impl->RotationEnabled = rotationEnabled;
}

void mitk::BoundingShapeInteractor::DataNodeChanged()
{
  mitk::DataNode::Pointer newInputNode = this->GetDataNode();

  if (newInputNode == nullptr)
    return;

  // add color properties
  mitk::ColorProperty::Pointer selectedColor =
    dynamic_cast<mitk::ColorProperty *>(newInputNode->GetProperty(selectedColorPropertyName));
  mitk::ColorProperty::Pointer deselectedColor =
    dynamic_cast<mitk::ColorProperty *>(newInputNode->GetProperty(deselectedColorPropertyName));

  if (selectedColor.IsNull())
    newInputNode->AddProperty(selectedColorPropertyName, mitk::ColorProperty::New(0.0, 1.0, 0.0));

  if (deselectedColor.IsNull())
    newInputNode->AddProperty(deselectedColorPropertyName, mitk::ColorProperty::New(1.0, 1.0, 1.0));

  newInputNode->SetProperty(boundingShapePropertyName, mitk::BoolProperty::New(true));
  newInputNode->AddProperty(activeHandleIdPropertyName, mitk::IntProperty::New(-1));
  newInputNode->SetProperty("layer", mitk::IntProperty::New(101));
  newInputNode->SetBoolProperty("fixedLayer", mitk::BoolProperty::New(true));
  newInputNode->SetBoolProperty("pickable", true);

  mitk::ColorProperty::Pointer initialColor =
    dynamic_cast<mitk::ColorProperty *>(newInputNode->GetProperty(deselectedColorPropertyName));
  if (initialColor.IsNotNull())
  {
    newInputNode->SetColor(initialColor->GetColor());
  }

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void mitk::BoundingShapeInteractor::HandlePositionChanged(const InteractionEvent *interactionEvent, Point3D &center)
{
  GeometryData::Pointer geometryData = dynamic_cast<GeometryData *>(this->GetDataNode()->GetData());
  int timeStep = interactionEvent->GetSender()->GetTimeStep(this->GetDataNode()->GetData());
  mitk::BaseGeometry::Pointer geometry = geometryData->GetGeometry(timeStep);

  std::vector<Point3D> cornerPoints = GetCornerPoints(geometry, true);
  if (m_Impl->Handles.size() == 6)
  {
    // set handle positions
    Point3D pointLeft = CalcAvgPoint(cornerPoints[5], cornerPoints[6]);
    Point3D pointRight = CalcAvgPoint(cornerPoints[1], cornerPoints[2]);
    Point3D pointTop = CalcAvgPoint(cornerPoints[0], cornerPoints[6]);
    Point3D pointBottom = CalcAvgPoint(cornerPoints[7], cornerPoints[1]);
    Point3D pointFront = CalcAvgPoint(cornerPoints[2], cornerPoints[7]);
    Point3D pointBack = CalcAvgPoint(cornerPoints[4], cornerPoints[1]);

    m_Impl->Handles[0].SetPosition(pointLeft);
    m_Impl->Handles[1].SetPosition(pointRight);
    m_Impl->Handles[2].SetPosition(pointTop);
    m_Impl->Handles[3].SetPosition(pointBottom);
    m_Impl->Handles[4].SetPosition(pointFront);
    m_Impl->Handles[5].SetPosition(pointBack);

    // calculate center based on half way of the distance between two opposing cornerpoints
    center = CalcAvgPoint(cornerPoints[7], cornerPoints[0]);
  }
}

void mitk::BoundingShapeInteractor::SetDataNode(DataNode *node)
{
  this->RestoreNodeProperties(); // if there is another node set, restore it's color

  if (node == nullptr)
    return;
  DataInteractor::SetDataNode(node); // calls DataNodeChanged internally
  this->DataNodeChanged();
}

bool mitk::BoundingShapeInteractor::CheckOverObject(const InteractionEvent *interactionEvent)
{
  const auto *positionEvent = dynamic_cast<const InteractionPositionEvent *>(interactionEvent);
  if (positionEvent == nullptr)
    return false;

  GeometryData::Pointer geometryData = dynamic_cast<GeometryData *>(this->GetDataNode()->GetData());
  int timeStep = interactionEvent->GetSender()->GetTimeStep(this->GetDataNode()->GetData());
  BaseGeometry::Pointer geometry = geometryData->GetGeometry(timeStep);

  // calculates translation based on offset+extent not on the transformation matrix (because the cube is located in the
  // center not in the origin)
  vtkSmartPointer<vtkMatrix4x4> imageTransform = geometry->GetVtkTransform()->GetMatrix();
  Point3D center = geometry->GetCenter();
  auto translation = vtkSmartPointer<vtkTransform>::New();
  auto transform = vtkSmartPointer<vtkTransform>::New();
  translation->Translate(center[0] - imageTransform->GetElement(0, 3),
                         center[1] - imageTransform->GetElement(1, 3),
                         center[2] - imageTransform->GetElement(2, 3));
  transform->SetMatrix(imageTransform);
  transform->PostMultiply();
  transform->Concatenate(translation);
  transform->Update();

  mitk::Vector3D extent;
  for (unsigned int i = 0; i < 3; ++i)
    extent[i] = (geometry->GetExtent(i));

  Point3D currentWorldPosition;
  Point2D currentDisplayPosition = positionEvent->GetPointerPositionOnScreen();
  interactionEvent->GetSender()->DisplayToWorld(currentDisplayPosition, currentWorldPosition);

  ScalarType transformedPosition[4];
  transformedPosition[0] = currentWorldPosition[0];
  transformedPosition[1] = currentWorldPosition[1];
  transformedPosition[2] = currentWorldPosition[2];
  transformedPosition[3] = 1;
  // transform point from world to object coordinates
  transform->GetInverse()->TransformPoint(transformedPosition, transformedPosition);
  // check if the world point is within bounds
  bool isInside = (transformedPosition[0] >= (-extent[0] / 2.0)) && (transformedPosition[0] <= (extent[0] / 2.0)) &&
                  (transformedPosition[1] >= (-extent[1] / 2.0)) && (transformedPosition[1] <= (extent[1] / 2.0)) &&
                  (transformedPosition[2] >= (-extent[2] / 2.0)) && (transformedPosition[2] <= (extent[2] / 2.0));

  return isInside;
}

bool mitk::BoundingShapeInteractor::CheckOverHandles(const InteractionEvent *interactionEvent)
{
  Point3D boundingBoxCenter;
  HandlePositionChanged(interactionEvent, boundingBoxCenter);
  const auto *positionEvent = dynamic_cast<const InteractionPositionEvent *>(interactionEvent);
  if (positionEvent == nullptr)
    return false;

  Point2D displayCenterPoint;
  // to do: change to actual time step (currently not necessary because geometry remains the same for each timestep
  int timeStep = 0;

  GeometryData::Pointer geometryData = dynamic_cast<GeometryData *>(this->GetDataNode()->GetData());
  BaseGeometry::Pointer geometry = geometryData->GetUpdatedTimeGeometry()->GetGeometryForTimeStep(timeStep);
  std::vector<Point3D> cornerPoints = GetCornerPoints(geometry, true);
  interactionEvent->GetSender()->WorldToDisplay(boundingBoxCenter, displayCenterPoint);
  double scale = interactionEvent->GetSender()->GetScaleFactorMMPerDisplayUnit(); // GetDisplaySizeInMM
  mitk::DoubleProperty::Pointer handleSizeProperty =
    dynamic_cast<mitk::DoubleProperty *>(this->GetDataNode()->GetProperty("Bounding Shape.Handle Size Factor"));

  ScalarType initialHandleSize;
  if (handleSizeProperty != nullptr)
    initialHandleSize = handleSizeProperty->GetValue();
  else
    initialHandleSize = 1.0 / 40.0;

  mitk::Point2D displaysize = interactionEvent->GetSender()->GetDisplaySizeInMM();
  ScalarType handlesize = ((displaysize[0] + displaysize[1]) / 2.0) * initialHandleSize;
  unsigned int handleNum = 0;

  for (auto &handle : m_Impl->Handles)
  {
    Point2D centerpoint;
    interactionEvent->GetSender()->WorldToDisplay(handle.GetPosition(), centerpoint);
    Point2D currentDisplayPosition = positionEvent->GetPointerPositionOnScreen();

    if ((currentDisplayPosition.EuclideanDistanceTo(centerpoint) < (handlesize / scale)) &&
        (currentDisplayPosition.EuclideanDistanceTo(displayCenterPoint) >
         (handlesize / scale))) // check if mouse is hovering over center point
    {
      handle.SetActive(true);
      m_Impl->ActiveHandle = handle;
      this->GetDataNode()->GetPropertyList()->SetProperty(activeHandleIdPropertyName,
                                                          mitk::IntProperty::New(handleNum++));
      this->GetDataNode()->GetData()->Modified();
      RenderingManager::GetInstance()->RequestUpdateAll();
      return true;
    }
    else
    {
      handleNum++;
      handle.SetActive(false);
    }
    this->GetDataNode()->GetPropertyList()->SetProperty(activeHandleIdPropertyName, mitk::IntProperty::New(-1));
  }

  return false;
}

void mitk::BoundingShapeInteractor::SelectHandle(StateMachineAction *, InteractionEvent *)
{
  this->DisableOriginalInteraction();
  DataNode::Pointer node = this->GetDataNode();

  if (node.IsNull())
    return;

  mitk::ColorProperty::Pointer selectedColor =
    dynamic_cast<mitk::ColorProperty *>(node->GetProperty(deselectedColorPropertyName));
  if (selectedColor.IsNotNull())
  {
    this->GetDataNode()->GetPropertyList()->SetProperty("color", selectedColor);
  }
  this->GetDataNode()->GetData()->UpdateOutputInformation(); // Geometry is up-to-date
  this->GetDataNode()->GetData()->Modified();
  RenderingManager::GetInstance()->RequestUpdateAll();
  return;
}

void mitk::BoundingShapeInteractor::DeselectHandles(StateMachineAction *, InteractionEvent *)
{
  this->DisableOriginalInteraction();
  DataNode::Pointer node = this->GetDataNode();

  if (node.IsNull())
    return;

  this->GetDataNode()->GetPropertyList()->SetProperty(activeHandleIdPropertyName, mitk::IntProperty::New(-1));
  this->GetDataNode()->GetData()->UpdateOutputInformation(); // Geometry is up-to-date
  this->GetDataNode()->GetData()->Modified();
  RenderingManager::GetInstance()->RequestUpdateAll();

  return;
}

void mitk::BoundingShapeInteractor::SelectObject(StateMachineAction *, InteractionEvent *)
{
  this->DisableOriginalInteraction(); // disable crosshair interaction and scolling if user is hovering over the object
  DataNode::Pointer node = this->GetDataNode();

  if (node.IsNull())
    return;

  mitk::ColorProperty::Pointer selectedColor =
    dynamic_cast<mitk::ColorProperty *>(node->GetProperty(selectedColorPropertyName));
  if (selectedColor.IsNotNull())
  {
    node->GetPropertyList()->SetProperty("color", selectedColor);
  }
  this->GetDataNode()->GetData()->UpdateOutputInformation(); // Geometry is up-to-date
  this->GetDataNode()->GetData()->Modified();
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  return;
}

void mitk::BoundingShapeInteractor::DeselectObject(StateMachineAction *, InteractionEvent *)
{
  this->EnableOriginalInteraction(); // enable crosshair interaction and scolling if user is hovering over the object

  DataNode::Pointer node = this->GetDataNode();

  if (node.IsNull())
    return;

  mitk::ColorProperty::Pointer deselectedColor =
    dynamic_cast<mitk::ColorProperty *>(node->GetProperty(deselectedColorPropertyName));
  if (deselectedColor.IsNotNull())
  {
    node->GetPropertyList()->SetProperty("color", deselectedColor);
  }

  this->GetDataNode()->GetData()->UpdateOutputInformation(); // Geometry is up-to-date
  this->GetDataNode()->GetData()->Modified();
  RenderingManager::GetInstance()->RequestUpdateAll();
  return;
}

void mitk::BoundingShapeInteractor::InitInteraction(StateMachineAction *, InteractionEvent *interactionEvent)
{
  InitMembers(interactionEvent);
}

bool mitk::BoundingShapeInteractor::InitMembers(InteractionEvent *interactionEvent)
{
  auto *positionEvent = dynamic_cast<InteractionPositionEvent *>(interactionEvent);
  if (positionEvent == nullptr)
    return false;

  // get initial position coordinates
  m_Impl->InitialPickedDisplayPoint = positionEvent->GetPointerPositionOnScreen();
  m_Impl->InitialPickedWorldPoint = positionEvent->GetPositionInWorld();
  m_Impl->LastPickedWorldPoint = positionEvent->GetPositionInWorld();

  return true;
}

void mitk::BoundingShapeInteractor::TranslateObject(StateMachineAction *, InteractionEvent *interactionEvent)
{
  auto *positionEvent = dynamic_cast<InteractionPositionEvent *>(interactionEvent);
  if (positionEvent == nullptr)
    return;

  int timeStep = interactionEvent->GetSender()->GetTimeStep(this->GetDataNode()->GetData());
  mitk::BaseGeometry::Pointer geometry =
    this->GetDataNode()->GetData()->GetUpdatedTimeGeometry()->GetGeometryForTimeStep(timeStep);
  Vector3D spacing = geometry->GetSpacing();
  Point3D currentPickedPoint;
  interactionEvent->GetSender()->DisplayToWorld(positionEvent->GetPointerPositionOnScreen(), currentPickedPoint);
  Vector3D interactionMove;

  // pixel aligned shifting of the bounding box
  interactionMove[0] = std::round((currentPickedPoint[0] - m_Impl->LastPickedWorldPoint[0]) / spacing[0]) * spacing[0];
  interactionMove[1] = std::round((currentPickedPoint[1] - m_Impl->LastPickedWorldPoint[1]) / spacing[1]) * spacing[1];
  interactionMove[2] = std::round((currentPickedPoint[2] - m_Impl->LastPickedWorldPoint[2]) / spacing[2]) * spacing[2];

  if ((interactionMove[0] + interactionMove[1] + interactionMove[2]) !=
      0.0) // only update current position if a movement occured
  {
    m_Impl->LastPickedWorldPoint = currentPickedPoint;

    geometry->SetOrigin(geometry->GetOrigin() + interactionMove);

    this->GetDataNode()->GetData()->UpdateOutputInformation(); // Geometry is up-to-date
    this->GetDataNode()->GetData()->Modified();
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
  return;
}

void mitk::BoundingShapeInteractor::ScaleObject(StateMachineAction *, InteractionEvent *interactionEvent)
{
  auto *positionEvent = dynamic_cast<InteractionPositionEvent *>(interactionEvent);
  if (positionEvent == nullptr)
    return;

  GeometryData::Pointer geometryData = dynamic_cast<GeometryData *>(this->GetDataNode()->GetData());
  Point3D handlePickedPoint = m_Impl->ActiveHandle.GetPosition();
  Point3D currentPickedPoint;
  interactionEvent->GetSender()->DisplayToWorld(positionEvent->GetPointerPositionOnScreen(), currentPickedPoint);
  int timeStep = interactionEvent->GetSender()->GetTimeStep(this->GetDataNode()->GetData());
  mitk::BaseGeometry::Pointer geometry = geometryData->GetGeometry(timeStep);
  Vector3D spacing = geometry->GetSpacing();

  // pixel aligned bounding box
  Vector3D interactionMove;
  interactionMove[0] = (currentPickedPoint[0] - m_Impl->LastPickedWorldPoint[0]);
  interactionMove[1] = (currentPickedPoint[1] - m_Impl->LastPickedWorldPoint[1]);
  interactionMove[2] = (currentPickedPoint[2] - m_Impl->LastPickedWorldPoint[2]);

  std::vector<int> faces = m_Impl->ActiveHandle.GetFaceIndices();
  auto pointscontainer = mitk::BoundingBox::PointsContainer::New();

  // calculate cornerpoints from geometry plus visualization offset
  std::vector<Point3D> cornerPoints = GetCornerPoints(geometry, true);
  unsigned int num = 0;
  for (const auto &point : cornerPoints)
  {
    pointscontainer->InsertElement(num++, point);
  }

  // calculate center based on half way of the distance between two opposing cornerpoints
  mitk::Point3D center = CalcAvgPoint(cornerPoints[7], cornerPoints[0]);

  Vector3D faceNormal;
  faceNormal[0] = handlePickedPoint[0] - center[0];
  faceNormal[1] = handlePickedPoint[1] - center[1];
  faceNormal[2] = handlePickedPoint[2] - center[2];
  Vector3D faceShift = ((faceNormal * interactionMove) / (faceNormal.GetNorm() * faceNormal.GetNorm())) * faceNormal;

  // calculate cornerpoints from geometry without visualization offset to update actual geometry
  cornerPoints = GetCornerPoints(geometry, false);
  num = 0;
  for (const auto &point : cornerPoints)
  {
    pointscontainer->InsertElement(num++, point);
  }

  bool positionChangeThreshold = true;
  for (int numFaces = 0; numFaces < 8; numFaces++) // estimate the corresponding face and shift its assigned points
  {
    if ((numFaces != faces[0]) && (numFaces != faces[1]) && (numFaces != faces[2]) && (numFaces != faces[3]))
    {
      Point3D point = pointscontainer->GetElement(numFaces);
      if (m_Impl->RotationEnabled) // apply if geometry is rotated and a pixel aligned shift is not possible
      {
        point[0] += faceShift[0];
        point[1] += faceShift[1];
        point[2] += faceShift[2];
      }
      else // shift pixelwise
      {
        point[0] += std::round(faceShift[0] / spacing[0]) * spacing[0];
        point[1] += std::round(faceShift[1] / spacing[1]) * spacing[1];
        point[2] += std::round(faceShift[2] / spacing[2]) * spacing[2];
      }

      if (point == pointscontainer->GetElement(numFaces))
        positionChangeThreshold = false;
      else
        m_Impl->LastPickedWorldPoint = point;

      pointscontainer->InsertElement(numFaces, point);
    }
  }

  if (positionChangeThreshold) // update only if bounding box is shifted at least by one pixel
  {
    auto inverse = mitk::AffineTransform3D::New();
    geometry->GetIndexToWorldTransform()->GetInverse(inverse);
    for (unsigned int pointid = 0; pointid < 8; pointid++)
    {
      pointscontainer->InsertElement(pointid, inverse->TransformPoint(pointscontainer->GetElement(pointid)));
    }

    auto bbox = mitk::BoundingBox::New();
    bbox->SetPoints(pointscontainer);
    bbox->ComputeBoundingBox();
    mitk::Point3D BBmin = bbox->GetMinimum();
    mitk::Point3D BBmax = bbox->GetMaximum();
    if (std::abs(BBmin[0] - BBmax[0]) > 0.01 && std::abs(BBmin[1] - BBmax[1]) > 0.01 &&
        std::abs(BBmin[2] - BBmax[2]) > 0.01) // TODO: check if the extent is greater than zero
    {
      geometry->SetBounds(bbox->GetBounds());
      geometry->Modified();
      this->GetDataNode()->GetData()->UpdateOutputInformation(); // Geometry is up-to-date
      this->GetDataNode()->GetData()->Modified();
      mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    }
  }
  return;
}

void mitk::BoundingShapeInteractor::RestoreNodeProperties()
{
  mitk::DataNode::Pointer inputNode = this->GetDataNode();
  if (inputNode.IsNull())
    return;

  mitk::ColorProperty::Pointer color = (mitk::ColorProperty::New(1.0, 1.0, 1.0));
  if (color.IsNotNull())
  {
    inputNode->GetPropertyList()->SetProperty("color", color);
  }
  inputNode->SetProperty("layer", mitk::IntProperty::New(99));
  inputNode->SetProperty(boundingShapePropertyName, mitk::BoolProperty::New(false));
  inputNode->GetPropertyList()->DeleteProperty(activeHandleIdPropertyName);

  EnableOriginalInteraction();
  // update rendering
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void mitk::BoundingShapeInteractor::EnableOriginalInteraction()
{
  // Re-enabling InteractionEventObservers that have been previously disabled for legacy handling of Tools
  // in new interaction framework
  for (const auto& displayInteractionConfig : m_Impl->DisplayInteractionConfigs)
  {
    if (displayInteractionConfig.first)
    {
      auto displayActionEventBroadcast = static_cast<mitk::DisplayActionEventBroadcast *>(
        us::GetModuleContext()->GetService<mitk::InteractionEventObserver>(displayInteractionConfig.first));

      if (nullptr != displayActionEventBroadcast)
      {
        // here the regular configuration is loaded again
        displayActionEventBroadcast->SetEventConfig(displayInteractionConfig.second);
      }
    }
  }

  m_Impl->DisplayInteractionConfigs.clear();
  m_Impl->OriginalInteractionEnabled = true;
}

void mitk::BoundingShapeInteractor::DisableOriginalInteraction()
{
  // dont deactivate twice, else we will clutter the config list ...
  if (false == m_Impl->OriginalInteractionEnabled)
    return;

  // As a legacy solution the display interaction of the new interaction framework is disabled here to avoid conflicts
  // with tools
  // Note: this only affects InteractionEventObservers (formerly known as Listeners) all DataNode specific interaction
  // will still be enabled
  m_Impl->DisplayInteractionConfigs.clear();
  auto eventObservers = us::GetModuleContext()->GetServiceReferences<mitk::InteractionEventObserver>();
  for (const auto& eventObserver : eventObservers)
  {
    auto *displayActionEventBroadcast = dynamic_cast<mitk::DisplayActionEventBroadcast *>(
      us::GetModuleContext()->GetService<mitk::InteractionEventObserver>(eventObserver));
    if (nullptr != displayActionEventBroadcast)
    {
      // remember the original configuration
      m_Impl->DisplayInteractionConfigs.insert(std::make_pair(eventObserver, displayActionEventBroadcast->GetEventConfig()));
      // here the alternative configuration is loaded
      displayActionEventBroadcast->AddEventConfig("DisplayConfigBlockLMB.xml");
    }
  }

  m_Impl->OriginalInteractionEnabled = false;
}
