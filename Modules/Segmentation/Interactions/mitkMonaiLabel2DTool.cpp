/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

// MITK
#include "mitkMonaiLabel2DTool.h"
#include <mitkIOUtil.h>
#include <mitkImageReadAccessor.h>

// us
#include <usGetModuleContext.h>
#include <usModule.h>
#include <usModuleContext.h>
#include <usModuleResource.h>
#include <usServiceReference.h>
#include <mitkSegTool2D.h>

namespace mitk
{
  MITK_TOOL_MACRO(MITKSEGMENTATION_EXPORT, MonaiLabel2DTool, "MonaiLabel2D");
}

void mitk::MonaiLabel2DTool::Activated()
{
  Superclass::Activated();
  this->SetLabelTransferScope(LabelTransferScope::AllLabels);
}

const char **mitk::MonaiLabel2DTool::GetXPM() const
{
  return nullptr;
}

us::ModuleResource mitk::MonaiLabel2DTool::GetIconResource() const
{
  us::Module *module = us::GetModuleContext()->GetModule();
  us::ModuleResource resource = module->GetResource("AI.svg");
  return resource;
}

const char *mitk::MonaiLabel2DTool::GetName() const
{
  return "MONAI Label 2D";
}

void mitk::MonaiLabel2DTool::OnAddPositivePoint(StateMachineAction *, InteractionEvent *interactionEvent)
{
  if ("deepgrow" == m_RequestParameters->model.type || "deepedit" == m_RequestParameters->model.type)
  {
    if ((nullptr == this->GetWorkingPlaneGeometry()) ||
        !mitk::Equal(*(interactionEvent->GetSender()->GetCurrentWorldPlaneGeometry()),
                     *(this->GetWorkingPlaneGeometry())))
    {
      this->ClearSeeds();
      this->SetWorkingPlaneGeometry(interactionEvent->GetSender()->GetCurrentWorldPlaneGeometry()->Clone());
      this->ResetPreviewContentAtTimeStep(
        RenderingManager::GetInstance()->GetTimeNavigationController()->GetSelectedTimeStep());
    }
    if (!this->IsUpdating() && m_PointSetPositive.IsNotNull())
    {
      const auto positionEvent = dynamic_cast<mitk::InteractionPositionEvent *>(interactionEvent);
      if (positionEvent != nullptr)
      {
        m_PointSetPositive->InsertPoint(m_PointSetCount, positionEvent->GetPositionInWorld());
        ++m_PointSetCount;
        this->UpdatePreview();
      }
    }
  }
}

void mitk::MonaiLabel2DTool::OnAddNegativePoint(StateMachineAction *, InteractionEvent *interactionEvent)
{
  if ("deepgrow" != m_RequestParameters->model.type)
  {
    return;
  }
  if (m_PointSetPositive->GetSize() == 0 || nullptr == this->GetWorkingPlaneGeometry() ||
      !mitk::Equal(*(interactionEvent->GetSender()->GetCurrentWorldPlaneGeometry()),
                   *(this->GetWorkingPlaneGeometry())))
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

void mitk::MonaiLabel2DTool::WriteImage(const Image *inputAtTimeStep, std::string &inputImagePath)
{
  Image::Pointer extendedImage = Image::New();
  std::array<unsigned int, 3> dim = {inputAtTimeStep->GetDimension(0), inputAtTimeStep->GetDimension(1), 1};
  mitk::PixelType pt = inputAtTimeStep->GetPixelType();
  extendedImage->Initialize(pt, 3, dim.data());

  ImageReadAccessor readAccessor(inputAtTimeStep);
  extendedImage->SetVolume(readAccessor.GetData());

  IOUtil::Save(extendedImage.GetPointer(), inputImagePath);
}

std::stringstream mitk::MonaiLabel2DTool::GetPointsAsListString(const mitk::BaseGeometry *baseGeometry,
                                                                PointSet::Pointer pointSet)
{
  std::stringstream pointsAndLabels;
  pointsAndLabels << "[";
  auto pointSetItPos = pointSet->Begin();
  const char COMMA = ',';
  while (pointSetItPos != pointSet->End())
  {
    auto point3d = pointSetItPos.Value();
    if (baseGeometry->IsInside(point3d))
    {
      Point3D index3D;
      baseGeometry->WorldToIndex(point3d, index3D);
      pointsAndLabels << "[";
      pointsAndLabels << static_cast<int>(index3D[0]) << COMMA << static_cast<int>(index3D[1]) << COMMA << 0 << "],";
    }
    ++pointSetItPos;
  }
  if (pointsAndLabels.tellp() > 1)
  {
    pointsAndLabels.seekp(-1, pointsAndLabels.end); // remove last added comma character
  }
  pointsAndLabels << "]";
  return pointsAndLabels;
}

void mitk::MonaiLabel2DTool::WriteBackResults(LabelSetImage *previewImage,
                                              LabelSetImage *segResults,
                                              TimeStepType timeStep)
{
  mitk::SegTool2D::WriteSliceToVolume(previewImage, this->GetWorkingPlaneGeometry(), segResults, timeStep, false);
}
