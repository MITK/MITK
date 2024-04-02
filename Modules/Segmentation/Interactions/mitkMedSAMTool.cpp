/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkMedSAMTool.h"

#include "mitkToolManager.h"
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
  CONNECT_FUNCTION("DeletePoint", OnDelete);
}

void mitk::MedSAMTool::OnRenderWindowClicked(StateMachineAction *, InteractionEvent *interactionEvent) 
{
  if (!this->GetIsReady())
  {
    return;
  }
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

void mitk::MedSAMTool::OnDelete(StateMachineAction*, InteractionEvent*)
{
  this->ClearPicks();
}

void mitk::MedSAMTool::ClearPicks() 
{
  this->GetDataStorage()->Remove(m_BoundingBoxNode);
  m_BoundingBoxNode = nullptr;
}

std::stringstream mitk::MedSAMTool::GetPointsAsCSVString(const mitk::BaseGeometry * /*baseGeometry*/)
{
  auto geometry = m_BoundingBoxNode->GetData()->GetGeometry();
  mitk::BoundingBox::ConstPointer boundingBox = geometry->GetBoundingBox();
  mitk::Point3D BBmin = boundingBox->GetMinimum();
  mitk::Point3D BBmax = boundingBox->GetMaximum();
  std::stringstream pointsAndLabels;
  pointsAndLabels << "Coordinates\n";
  const char SPACE = ' ';
  pointsAndLabels << abs(static_cast<int>(BBmin[0])) << SPACE << abs(static_cast<int>(BBmin[1])) << SPACE
                  << abs(static_cast<int>(BBmax[0])) << SPACE << abs(static_cast<int>(BBmax[1]));
  return pointsAndLabels;
}
