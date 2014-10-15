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
#include "mitkImage.h"
#include "mitkToolManager.h"
#include "mitkProperties.h"
// us
#include <usModule.h>
#include <usModuleResource.h>
#include <usGetModuleContext.h>
#include <usModuleContext.h>


namespace mitk {
  MITK_TOOL_MACRO(MitkSegmentation_EXPORT, AdaptiveRegionGrowingTool, "AdaptiveRegionGrowingTool");
}

mitk::AdaptiveRegionGrowingTool::AdaptiveRegionGrowingTool()
{
  m_PointSetNode = mitk::DataNode::New();
  m_PointSetNode->GetPropertyList()->SetProperty("name", mitk::StringProperty::New("3D_Regiongrowing_Seedpoint"));
  m_PointSetNode->GetPropertyList()->SetProperty("helper object", mitk::BoolProperty::New(true));
  m_PointSet = mitk::PointSet::New();
  m_PointSetNode->SetData(m_PointSet);
  m_SeedPointInteractor = mitk::SinglePointDataInteractor::New();
  m_SeedPointInteractor->LoadStateMachine("PointSet.xml");
  m_SeedPointInteractor->SetEventConfig("PointSetConfig.xml");
  m_SeedPointInteractor->SetDataNode(m_PointSetNode);
}

mitk::AdaptiveRegionGrowingTool::~AdaptiveRegionGrowingTool()
{
}

bool mitk::AdaptiveRegionGrowingTool::CanHandle(BaseData* referenceData) const
{
  if (referenceData == NULL)
    return false;

  Image* image = dynamic_cast<Image*>(referenceData);

  if (image == NULL)
    return false;

  if (image->GetDimension() < 3)
    return false;

  return true;
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
}

void mitk::AdaptiveRegionGrowingTool::Deactivated()
{
  m_PointSet->Clear();
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
