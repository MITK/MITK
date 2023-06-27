/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkSegmentAnythingTool.h"

#include "mitkInteractionPositionEvent.h"
#include "mitkPointSetShapeProperty.h"
#include "mitkProperties.h"
#include "mitkToolManager.h"
// us
#include "mitkSegTool2D.h"
#include <chrono>
#include <thread>
#include <usGetModuleContext.h>
#include <usModule.h>
#include <usModuleContext.h>
#include <usModuleResource.h>

using namespace std::chrono_literals;

namespace mitk
{
  MITK_TOOL_MACRO(MITKSEGMENTATION_EXPORT, SegmentAnythingTool, "SegmentAnythingTool");
}

mitk::SegmentAnythingTool::SegmentAnythingTool() : SegWithPreviewTool(true, "PressMoveReleaseAndPointSetting")
{
  this->ResetsToEmptyPreviewOn();
  this->IsTimePointChangeAwareOff();
  this->KeepActiveAfterAcceptOn();
}

const char **mitk::SegmentAnythingTool::GetXPM() const
{
  return nullptr;
}

const char *mitk::SegmentAnythingTool::GetName() const
{
  return "Segment Anything";
}

us::ModuleResource mitk::SegmentAnythingTool::GetIconResource() const
{
  us::Module *module = us::GetModuleContext()->GetModule();
  us::ModuleResource resource = module->GetResource("AI.svg");
  return resource;
}

void mitk::SegmentAnythingTool::Activated()
{
  Superclass::Activated();
  m_PointSetPositive = mitk::PointSet::New();
  m_PointSetNodePositive = mitk::DataNode::New();
  m_PointSetNodePositive->SetData(m_PointSetPositive);
  m_PointSetNodePositive->SetName(std::string(this->GetName()) + "_PointSetPositive");
  m_PointSetNodePositive->SetBoolProperty("helper object", true);
  m_PointSetNodePositive->SetColor(0.0, 1.0, 0.0);
  m_PointSetNodePositive->SetVisibility(true);
  m_PointSetNodePositive->SetProperty("Pointset.2D.shape",
                              mitk::PointSetShapeProperty::New(mitk::PointSetShapeProperty::CIRCLE));
  m_PointSetNodePositive->SetProperty("Pointset.2D.fill shape", mitk::BoolProperty::New(true));
  this->GetDataStorage()->Add(m_PointSetNodePositive, this->GetToolManager()->GetWorkingData(0));

  m_PointSetNegative = mitk::PointSet::New();
  m_PointSetNodeNegative = mitk::DataNode::New();
  m_PointSetNodeNegative->SetData(m_PointSetNegative);
  m_PointSetNodeNegative->SetName(std::string(this->GetName()) + "_PointSetNegative");
  m_PointSetNodeNegative->SetBoolProperty("helper object", true);
  m_PointSetNodeNegative->SetColor(1.0, 0.0, 0.0);
  m_PointSetNodeNegative->SetVisibility(true);
  m_PointSetNodeNegative->SetProperty("Pointset.2D.shape",
                                      mitk::PointSetShapeProperty::New(mitk::PointSetShapeProperty::CIRCLE));
  m_PointSetNodeNegative->SetProperty("Pointset.2D.fill shape", mitk::BoolProperty::New(true));
  this->GetDataStorage()->Add(m_PointSetNodeNegative, this->GetToolManager()->GetWorkingData(0));

  this->SetLabelTransferScope(LabelTransferScope::ActiveLabel);
  this->SetLabelTransferMode(LabelTransferMode::MapLabel);
}

void mitk::SegmentAnythingTool::Deactivated()
{
  this->ClearSeeds();
  GetDataStorage()->Remove(m_PointSetNodePositive);
  GetDataStorage()->Remove(m_PointSetNodeNegative);
  m_PointSetNodePositive = nullptr;
  m_PointSetNodeNegative = nullptr;
  m_PointSetPositive = nullptr;
  m_PointSetNegative = nullptr;
  m_PythonService.reset();
  Superclass::Deactivated();
}

void mitk::SegmentAnythingTool::ConnectActionsAndFunctions()
{
  CONNECT_FUNCTION("ShiftSecondaryButtonPressed", OnAddNegativePoint);
  CONNECT_FUNCTION("ShiftPrimaryButtonPressed", OnAddPoint);
  CONNECT_FUNCTION("DeletePoint", OnDelete);
}

void mitk::SegmentAnythingTool::InitSAMPythonProcess()
{
  if (nullptr != m_PythonService)
  {
    m_PythonService.reset();
  }
  m_PythonService = std::make_unique<mitk::SegmentAnythingPythonService>(
    this->GetPythonPath(), this->GetModelType(), this->GetCheckpointPath(), this->GetGpuId());
  m_PythonService->StartAsyncProcess();
}

bool mitk::SegmentAnythingTool::IsPythonReady() const
{
  return m_PythonService->CheckStatus();
}

void mitk::SegmentAnythingTool::OnAddNegativePoint(StateMachineAction *, InteractionEvent *interactionEvent)
{
  if (!this->GetIsReady() || m_PointSetPositive->GetSize() == 0)
  {
    return;
  }
  if (!this->IsUpdating() && m_PointSetNegative.IsNotNull())
  {
    const auto positionEvent = dynamic_cast<mitk::InteractionPositionEvent *>(interactionEvent);
    if (positionEvent != nullptr)
    {
      m_PointSetNegative->InsertPoint(m_PointSetCount, positionEvent->GetPositionInWorld());
      m_PointSetCount++;
      this->UpdatePreview();
    }
  }
}

void mitk::SegmentAnythingTool::OnAddPoint(StateMachineAction *, InteractionEvent *interactionEvent)
{
  if (!this->GetIsReady())
  {
    return;
  }
  m_IsGenerateEmbeddings = false;
  if ((nullptr == this->GetWorkingPlaneGeometry()) ||
      !mitk::Equal(*(interactionEvent->GetSender()->GetCurrentWorldPlaneGeometry()),
                   *(this->GetWorkingPlaneGeometry())))
  {
    m_IsGenerateEmbeddings = true;
    this->ClearSeeds();
    this->SetWorkingPlaneGeometry(interactionEvent->GetSender()->GetCurrentWorldPlaneGeometry()->Clone());
  }
  if (!this->IsUpdating() && m_PointSetPositive.IsNotNull())
  {
    const auto positionEvent = dynamic_cast<mitk::InteractionPositionEvent *>(interactionEvent);
    if (positionEvent != nullptr)
    {
      m_PointSetPositive->InsertPoint(m_PointSetCount, positionEvent->GetPositionInWorld());
      m_PointSetCount++;
      this->UpdatePreview();
    }
  }
}

void mitk::SegmentAnythingTool::OnDelete(StateMachineAction *, InteractionEvent *)
{
  if (!this->IsUpdating() && m_PointSetPositive.IsNotNull())
  {
    PointSet::Pointer removeSet = m_PointSetPositive;
    decltype(m_PointSetPositive->GetMaxId().Index()) maxId = 0;
    if (m_PointSetPositive->GetSize() > 0)
    {
      maxId = m_PointSetPositive->GetMaxId().Index();
    }
    if (m_PointSetNegative->GetSize() > 0 && (maxId < m_PointSetNegative->GetMaxId().Index()))
    {
      removeSet = m_PointSetNegative;
    }
    removeSet->RemovePointAtEnd(0);
    --m_PointSetCount;
    this->UpdatePreview();
  }
}

void mitk::SegmentAnythingTool::ClearPicks()
{
  this->ClearSeeds();
  this->UpdatePreview();
}

bool mitk::SegmentAnythingTool::HasPicks() const
{
  return this->m_PointSetPositive.IsNotNull() && this->m_PointSetPositive->GetSize() > 0;
}

void mitk::SegmentAnythingTool::ClearSeeds()
{
  if (this->m_PointSetPositive.IsNotNull())
  {
    m_PointSetCount -= m_PointSetPositive->GetSize();
    this->m_PointSetPositive = mitk::PointSet::New(); // renew pointset
    this->m_PointSetNodePositive->SetData(this->m_PointSetPositive);
  }
  if (this->m_PointSetNegative.IsNotNull())
  {
    m_PointSetCount -= m_PointSetNegative->GetSize();
    this->m_PointSetNegative = mitk::PointSet::New(); // renew pointset
    this->m_PointSetNodeNegative->SetData(this->m_PointSetNegative);
  }
}

void mitk::SegmentAnythingTool::ConfirmCleanUp() 
{
  auto previewImage = this->GetPreviewSegmentation();
  for (unsigned int timeStep = 0; timeStep < previewImage->GetTimeSteps(); ++timeStep)
  {
    this->ResetPreviewContentAtTimeStep(timeStep);
  }
  this->ClearSeeds();
  RenderingManager::GetInstance()->RequestUpdateAll();
}

void mitk::SegmentAnythingTool::DoUpdatePreview(const Image *inputAtTimeStep,
                                                const Image * /*oldSegAtTimeStep*/,
                                                LabelSetImage *previewImage,
                                                TimeStepType timeStep)
{
  if (nullptr != previewImage && m_PointSetPositive.IsNotNull())
  {
    if (this->HasPicks() && nullptr != m_PythonService)
    {
      std::string uniquePlaneID = GetHashForCurrentPlane();
      try
      {
        this->EmitSAMStatusMessageEvent("Prompting Segment Anything Model...");
        m_PythonService->TransferImageToProcess(inputAtTimeStep, uniquePlaneID);
        auto csvStream = this->GetPointsAsCSVString(inputAtTimeStep->GetGeometry());
        m_ProgressCommand->SetProgress(100);
        m_PythonService->TransferPointsToProcess(csvStream);
        m_ProgressCommand->SetProgress(150);
        std::this_thread::sleep_for(100ms);
        mitk::LabelSetImage::Pointer outputBuffer = m_PythonService->RetrieveImageFromProcess();
        m_ProgressCommand->SetProgress(180);
        mitk::SegTool2D::WriteSliceToVolume(previewImage, this->GetWorkingPlaneGeometry(), outputBuffer.GetPointer(), timeStep, false);
        this->SetSelectedLabels({MASK_VALUE});
        this->EmitSAMStatusMessageEvent("Successfully generated segmentation.");
      }
      catch (const mitk::Exception &e)
      {
        this->EmitSAMStatusMessageEvent(e.GetDescription());
        mitkThrow() << e.GetDescription();
      }
    }
    else if (nullptr != this->GetWorkingPlaneGeometry())
    {
      this->ResetPreviewContentAtTimeStep(timeStep);
      RenderingManager::GetInstance()->ForceImmediateUpdateAll();
    }
  }
}

std::string mitk::SegmentAnythingTool::GetHashForCurrentPlane()
{
  mitk::Vector3D normal = this->GetWorkingPlaneGeometry()->GetNormal();
  std::stringstream hashstream;
  hashstream << normal[0] << normal[1] << normal[2];
  mitk::Point3D point = m_PointSetPositive->GetPoint(0);
  for (int i = 0; i < 3; ++i)
  {
    if (normal[i] != 0)
    {
      hashstream << point[i];
    }
  }
  size_t hashVal = std::hash<std::string>{}(hashstream.str());
  return std::to_string(hashVal);
}

std::stringstream mitk::SegmentAnythingTool::GetPointsAsCSVString(const mitk::BaseGeometry *baseGeometry)
{
  MITK_INFO << "No.of points: " << m_PointSetPositive->GetSize();
  std::stringstream pointsAndLabels;
  pointsAndLabels << "Point,Label\n";
  mitk::PointSet::PointsIterator pointSetItPos = m_PointSetPositive->Begin();
  mitk::PointSet::PointsIterator pointSetItNeg = m_PointSetNegative->Begin();
  const char SPACE = ' ';
  while (pointSetItPos != m_PointSetPositive->End() || pointSetItNeg != m_PointSetNegative->End())
  {
    if (pointSetItPos != m_PointSetPositive->End())
    {
      mitk::Point3D point = pointSetItPos.Value();
      if (baseGeometry->IsInside(point))
      {
        Point2D p2D = Get2DIndicesfrom3DWorld(baseGeometry, point);
        pointsAndLabels << (int)p2D[0] << SPACE << (int)p2D[1] << ",1" << std::endl;
      }
      ++pointSetItPos;
    }
    if (pointSetItNeg != m_PointSetNegative->End())
    {
      mitk::Point3D point = pointSetItNeg.Value();
      if (baseGeometry->IsInside(point))
      {
        Point2D p2D = Get2DIndicesfrom3DWorld(baseGeometry, point);
        pointsAndLabels << (int)p2D[0] << SPACE << (int)p2D[1] << ",0" << std::endl;
      }
      ++pointSetItNeg;
    }
  }
  return pointsAndLabels;
}

std::vector<std::pair<mitk::Point2D, std::string>> mitk::SegmentAnythingTool::GetPointsAsVector(
  const mitk::BaseGeometry *baseGeometry)
{
  std::vector<std::pair<mitk::Point2D, std::string>> clickVec;
  clickVec.reserve(m_PointSetPositive->GetSize() + m_PointSetNegative->GetSize());
  mitk::PointSet::PointsIterator pointSetItPos = m_PointSetPositive->Begin();
  mitk::PointSet::PointsIterator pointSetItNeg = m_PointSetNegative->Begin();
  while (pointSetItPos != m_PointSetPositive->End() || pointSetItNeg != m_PointSetNegative->End())
  {
    if (pointSetItPos != m_PointSetPositive->End())
    {
      mitk::Point3D point = pointSetItPos.Value();
      if (baseGeometry->IsInside(point))
      {
        Point2D p2D = Get2DIndicesfrom3DWorld(baseGeometry, point);
        clickVec.push_back(std::pair(p2D, "1"));
      }
      ++pointSetItPos;
    }
    if (pointSetItNeg != m_PointSetNegative->End())
    {
      mitk::Point3D point = pointSetItNeg.Value();
      if (baseGeometry->IsInside(point))
      {
        Point2D p2D = Get2DIndicesfrom3DWorld(baseGeometry, point);
        clickVec.push_back(std::pair(p2D, "0"));
      }
      ++pointSetItNeg;
    }
  }
  return clickVec;
}

mitk::Point2D mitk::SegmentAnythingTool::Get2DIndicesfrom3DWorld(const mitk::BaseGeometry *baseGeometry,
                                                                 mitk::Point3D &point3d)
{
  baseGeometry->WorldToIndex(point3d, point3d);
  MITK_INFO << point3d[0] << " " << point3d[1] << " " << point3d[2]; // remove
  Point2D point2D;
  point2D.SetElement(0, point3d[0]);
  point2D.SetElement(1, point3d[1]);
  return point2D;
}

void mitk::SegmentAnythingTool::EmitSAMStatusMessageEvent(const std::string& status)
{
  SAMStatusMessageEvent.Send(status);
}
