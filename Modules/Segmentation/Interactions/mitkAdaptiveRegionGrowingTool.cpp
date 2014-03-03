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

#include "mitkAdaptiveRegionGrowingTool.h"

#include "mitkToolManager.h"
#include "mitkProperties.h"
#include <mitkInteractionConst.h>
#include "mitkGlobalInteraction.h"

// us
#include <usModule.h>
#include <usModuleResource.h>
#include <usGetModuleContext.h>
#include <usModuleContext.h>


namespace mitk {
  MITK_TOOL_MACRO(Segmentation_EXPORT, AdaptiveRegionGrowingTool, "AdaptiveRegionGrowingTool");
}

mitk::AdaptiveRegionGrowingTool::AdaptiveRegionGrowingTool() : SegTool3D("dummy")
{
  m_PointSetNode = mitk::DataNode::New();
  m_PointSetNode->GetPropertyList()->SetProperty("name", mitk::StringProperty::New("3D_Regiongrowing_Seedpoint"));
  m_PointSetNode->GetPropertyList()->SetProperty("helper object", mitk::BoolProperty::New(true));
  m_PointSet = mitk::PointSet::New();
  m_PointSetNode->SetData(m_PointSet);
  m_SeedPointInteractor = mitk::PointSetInteractor::New("singlepointinteractor", m_PointSetNode);
}

mitk::AdaptiveRegionGrowingTool::~AdaptiveRegionGrowingTool()
{
}

const char** mitk::AdaptiveRegionGrowingTool::GetXPM() const
{
  return NULL;
}

const char* mitk::AdaptiveRegionGrowingTool::GetName() const
{
  return "Region Growing 3D";
}

us::ModuleResource mitk::AdaptiveRegionGrowingTool::GetIconResource() const
{
  us::Module* module = us::GetModuleContext()->GetModule();
  us::ModuleResource resource = module->GetResource("RegionGrowing_48x48.png");
  return resource;
}

void mitk::AdaptiveRegionGrowingTool::Activated()
{
  if (!GetDataStorage()->Exists(m_PointSetNode))
    GetDataStorage()->Add(m_PointSetNode, GetWorkingData());
  mitk::GlobalInteraction::GetInstance()->AddInteractor(m_SeedPointInteractor);
}

void mitk::AdaptiveRegionGrowingTool::Deactivated()
{
  if (m_PointSet->GetPointSet()->GetNumberOfPoints() != 0)
  {
    mitk::Point3D point = m_PointSet->GetPoint(0);
    mitk::PointOperation* doOp = new mitk::PointOperation(mitk::OpREMOVE, point, 0);
    m_PointSet->ExecuteOperation(doOp);
  }
  mitk::GlobalInteraction::GetInstance()->RemoveInteractor(m_SeedPointInteractor);
  GetDataStorage()->Remove(m_PointSetNode);
}

mitk::DataNode* mitk::AdaptiveRegionGrowingTool::GetReferenceData(){
  return this->m_ToolManager->GetReferenceData(0);
}

mitk::DataStorage* mitk::AdaptiveRegionGrowingTool::GetDataStorage(){
  return this->m_ToolManager->GetDataStorage();
}

mitk::DataNode* mitk::AdaptiveRegionGrowingTool::GetWorkingData(){
  return this->m_ToolManager->GetWorkingData(0);
}

mitk::DataNode::Pointer mitk::AdaptiveRegionGrowingTool::GetPointSetNode()
{
  return m_PointSetNode;
}
