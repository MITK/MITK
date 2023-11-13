/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkMonaiLabel3DTool.h"

#include <mitkIOUtil.h>
#include <usGetModuleContext.h>
#include <usModule.h>
#include <usModuleContext.h>
#include <usModuleResource.h>
#include <usServiceReference.h>
#include <mitkImageReadAccessor.h>

namespace mitk
{
  MITK_TOOL_MACRO(MITKSEGMENTATION_EXPORT, MonaiLabel3DTool, "MonaiLabel3D");
}

void mitk::MonaiLabel3DTool::Activated()
{
  Superclass::Activated();
  this->SetLabelTransferScope(LabelTransferScope::AllLabels);
}

const char **mitk::MonaiLabel3DTool::GetXPM() const
{
  return nullptr;
}

us::ModuleResource mitk::MonaiLabel3DTool::GetIconResource() const
{
  us::Module *module = us::GetModuleContext()->GetModule();
  us::ModuleResource resource = module->GetResource("AI.svg");
  return resource;
}

const char *mitk::MonaiLabel3DTool::GetName() const
{
  return "MONAI Label 3D";
}

void mitk::MonaiLabel3DTool::WriteImage(const Image *inputAtTimeStep, std::string &inputImagePath)
{
  IOUtil::Save(inputAtTimeStep, inputImagePath);
}

void mitk::MonaiLabel3DTool::WriteBackResults(LabelSetImage *previewImage,
                                              LabelSetImage *segResults,
                                              TimeStepType timeStep)
{
  mitk::ImageReadAccessor newMitkImgAcc(segResults);
  previewImage->SetVolume(newMitkImgAcc.GetData(), timeStep);
}

void mitk::MonaiLabel3DTool::OnAddPositivePoint(StateMachineAction *, InteractionEvent *interactionEvent)
{
  if ("deepgrow" == m_RequestParameters->model.type || "deepedit" == m_RequestParameters->model.type)
  {
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

void mitk::MonaiLabel3DTool::OnAddNegativePoint(StateMachineAction *, InteractionEvent *interactionEvent)
{
  if ("deepgrow" != m_RequestParameters->model.type)
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

std::stringstream mitk::MonaiLabel3DTool::GetPointsAsListString(const mitk::BaseGeometry *baseGeometry,
                                                              PointSet::Pointer pointSet)
{
  MITK_INFO << "No.of points: " << pointSet->GetSize();
  std::stringstream pointsAndLabels;
  pointsAndLabels << "[";
  mitk::PointSet::PointsConstIterator pointSetItPos = pointSet->Begin();
  const char COMMA = ',';
  while (pointSetItPos != pointSet->End())
  {
    mitk::Point3D point3d = pointSetItPos.Value();
    if (baseGeometry->IsInside(point3d))
    {
      mitk::Point3D index3D;
      baseGeometry->WorldToIndex(point3d, index3D);
      pointsAndLabels << "[";
      pointsAndLabels << static_cast<int>(index3D[0]) << COMMA << static_cast<int>(index3D[1]) << COMMA
                      << static_cast<int>(index3D[2]) << "],";
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

