/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkMedSAMTool.h"

//#include "mitkImageAccessByItk.h"
#include "mitkToolManager.h"
#include <mitkSegTool2D.h>
#include "mitkGeometryData.h"
#include "mitkInteractionPositionEvent.h"


// us
#include <usGetModuleContext.h>
#include <usModule.h>
#include <usModuleContext.h>
#include <usModuleResource.h>

using namespace std::chrono_literals;

namespace mitk
{
  MITK_TOOL_MACRO(MITKSEGMENTATION_EXPORT, MedSAMTool, "MedSAMTool");

  void FlipAxis(mitk::BaseGeometry *geometry, int axis)
  {
    auto matrix = geometry->GetVtkMatrix();
    matrix->SetElement(axis, axis, -matrix->GetElement(axis, axis));
    matrix->SetElement(axis, 3, matrix->GetElement(axis, 3) - geometry->GetExtentInMM(axis));

    geometry->SetIndexToWorldTransformByVtkMatrix(matrix);

    auto bounds = geometry->GetBounds();
    int minIndex = 2 * axis;
    bounds[minIndex] *= -1;
    bounds[minIndex + 1] += 2 * bounds[minIndex];

    geometry->SetBounds(bounds);
  }

  mitk::BaseGeometry::Pointer RectifyGeometry(const mitk::BaseGeometry *geometry)
  {
    auto rectifiedGeometry = geometry->Clone();
    auto matrix = rectifiedGeometry->GetVtkMatrix();

    for (int axis = 0; axis < 3; ++axis)
    {
      if (matrix->GetElement(axis, axis) < 0.0)
        FlipAxis(rectifiedGeometry, axis);
    }
    return rectifiedGeometry;
  }
}

const char *mitk::MedSAMTool::GetName() const
{
  return "MedSAM";
}

void mitk::MedSAMTool::Activated()
{
  SegWithPreviewTool::Activated();
  this->SetLabelTransferScope(LabelTransferScope::ActiveLabel);
  this->SetLabelTransferMode(LabelTransferMode::MapLabel);
}

void mitk::MedSAMTool::Deactivated()
{
  SegWithPreviewTool::Deactivated();
  GetDataStorage()->Remove(m_BoundingBoxNode);
  m_BoundingBoxNode = nullptr;
  m_PythonService.reset();
}

void mitk::MedSAMTool::ConnectActionsAndFunctions() 
{
  CONNECT_FUNCTION("ShiftPrimaryButtonPressed", OnRenderWindowClicked);
}

void mitk::MedSAMTool::OnRenderWindowClicked(StateMachineAction *, InteractionEvent *interactionEvent) 
{
  /* if (!this->GetIsReady())
  {
    return;
  }*/
  if ((nullptr == this->GetWorkingPlaneGeometry()) ||
      !mitk::Equal(*(interactionEvent->GetSender()->GetCurrentWorldPlaneGeometry()),
                   *(this->GetWorkingPlaneGeometry())))
  {
    this->GetDataStorage()->Remove(m_BoundingBoxNode);
    m_BoundingBoxNode = nullptr;
    this->SetWorkingPlaneGeometry(interactionEvent->GetSender()->GetCurrentWorldPlaneGeometry()->Clone());
    auto boundingBox = mitk::GeometryData::New();
    boundingBox->SetGeometry(static_cast<mitk::Geometry3D *>(
      this->InitializeWithImageGeometry(interactionEvent->GetSender()->GetCurrentWorldPlaneGeometry()->Clone())));
    m_BoundingBoxNode = mitk::DataNode::New();
    m_BoundingBoxNode->SetData(boundingBox);
    m_BoundingBoxNode->SetVisibility(true);
    m_BoundingBoxNode->SetName(std::string(this->GetName()) + "_Boundingbox");
    m_BoundingBoxNode->SetProperty("layer", mitk::IntProperty::New(99));
    m_BoundingBoxNode->AddProperty("Bounding Shape.Handle Size Factor", mitk::DoubleProperty::New(0.02));
    m_BoundingBoxNode->SetBoolProperty("pickable", true);
    this->GetDataStorage()->Add(m_BoundingBoxNode, this->GetToolManager()->GetWorkingData(0));
    this->CreateBoundingShapeInteractor(false);
    m_BoundingShapeInteractor->EnableInteraction(true);
    m_BoundingShapeInteractor->SetDataNode(m_BoundingBoxNode);
    mitk::RenderingManager::GetInstance()->InitializeViews();
  }
}

void mitk::MedSAMTool::CreateBoundingShapeInteractor(bool rotationEnabled)
{
  if (m_BoundingShapeInteractor.IsNull())
  {
    m_BoundingShapeInteractor = mitk::BoundingShapeInteractor::New();
    m_BoundingShapeInteractor->LoadStateMachine("BoundingShapeInteraction.xml",
                                                us::ModuleRegistry::GetModule("MitkBoundingShape"));
    m_BoundingShapeInteractor->SetEventConfig("BoundingShapeMouseConfig.xml",
                                              us::ModuleRegistry::GetModule("MitkBoundingShape"));
  }
  m_BoundingShapeInteractor->SetRotationEnabled(rotationEnabled);
}

mitk::Geometry3D::Pointer mitk::MedSAMTool::InitializeWithImageGeometry(const mitk::BaseGeometry *geometry) const
{
  // convert a BaseGeometry into a Geometry3D (otherwise IO is not working properly)
  if (geometry == nullptr)
    mitkThrow() << "Geometry is not valid.";

  auto boundingGeometry = mitk::Geometry3D::New();
  boundingGeometry->SetBounds(geometry->GetBounds());
  boundingGeometry->SetImageGeometry(geometry->GetImageGeometry());
  boundingGeometry->SetOrigin(geometry->GetOrigin());
  boundingGeometry->SetSpacing(geometry->GetSpacing());
  boundingGeometry->SetIndexToWorldTransform(geometry->GetIndexToWorldTransform()->Clone());
  boundingGeometry->Modified();
  return boundingGeometry;
}

bool mitk::MedSAMTool::HasPicks() const
{
  return m_BoundingBoxNode.IsNotNull();
}

void mitk::MedSAMTool::ClearPicks() 
{
  this->GetDataStorage()->Remove(m_BoundingBoxNode);
  m_BoundingBoxNode = nullptr;
}

std::stringstream mitk::MedSAMTool::GetPointsAsCSVString(const mitk::BaseGeometry * baseGeometry)
{
  //auto geometry = RectifyGeometry(m_BoundingBoxNode->GetData()->GetGeometry());
  //const auto origin = geometry->GetOrigin();
  //                    //- RectifyGeometry(this->GetToolManager()->GetWorkingData(0)->GetData()->GetGeometry())->GetOrigin();
  //const auto spacing = geometry->GetSpacing();
  //const auto bounds = geometry->GetBounds();
  //
  //MITK_INFO << "origin " << origin[0];
  //MITK_INFO << "origin " << origin[1];
  //MITK_INFO << "origin " << origin[2];

  //MITK_INFO << "bounds: " << bounds[0];
  //MITK_INFO << "bounds: " << bounds[1];
  //MITK_INFO << "bounds: " << bounds[2];
  //MITK_INFO << "bounds: " << bounds[3];
  //MITK_INFO << "bounds: " << bounds[4];
  //MITK_INFO << "bounds: " << bounds[5];


  //Point2D p2D_1 = this->Get2DIndicesfrom3DWorld(baseGeometry, origin);
  //Point2D p2D_2;
  //p2D_2[0] = p2D_1[0] - bounds[1];
  //p2D_2[1] = p2D_1[1] - bounds[3];

  //MITK_INFO << p2D_2[0] << " " << p2D_2[1]; // remove

  std::stringstream pointsAndLabels;
  pointsAndLabels << "Coordinates\n";
  //pointsAndLabels << "20 20 450 450";
  auto tmp = baseGeometry->GetBounds();
  int coordinate1 = static_cast<int>(tmp[1]*0.25);
  int coordinate2 = static_cast<int>(tmp[3]*0.75);
  const char SPACE = ' ';
  pointsAndLabels << coordinate1 << SPACE << coordinate1 << SPACE << coordinate2 << SPACE << coordinate2;
  //pointsAndLabels << p2D_2[1] << SPACE << p2D_2[0] << SPACE << p2D_1[2] << SPACE << p2D_1[1];
  return pointsAndLabels;
}
