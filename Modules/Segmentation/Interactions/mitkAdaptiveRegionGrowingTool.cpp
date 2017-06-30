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
#include "mitkProperties.h"
#include "mitkToolManager.h"
// us
#include <usGetModuleContext.h>
#include <usModule.h>
#include <usModuleContext.h>
#include <usModuleResource.h>

namespace mitk
{
  MITK_TOOL_MACRO(MITKSEGMENTATION_EXPORT, AdaptiveRegionGrowingTool, "AdaptiveRegionGrowingTool");
}

mitk::AdaptiveRegionGrowingTool::AdaptiveRegionGrowingTool()
{
  m_PointSetNode = mitk::DataNode::New();
  m_PointSetNode->GetPropertyList()->SetProperty("name", mitk::StringProperty::New("3D_Regiongrowing_Seedpoint"));
  m_PointSetNode->GetPropertyList()->SetProperty("helper object", mitk::BoolProperty::New(true));
  m_PointSet = mitk::PointSet::New();
  m_PointSetNode->SetData(m_PointSet);
}

mitk::AdaptiveRegionGrowingTool::~AdaptiveRegionGrowingTool()
{
}

bool mitk::AdaptiveRegionGrowingTool::CanHandle(BaseData *referenceData) const
{
  if (referenceData == nullptr)
    return false;

  Image *image = dynamic_cast<Image *>(referenceData);

  if (image == nullptr)
    return false;

  if (image->GetDimension() < 3)
    return false;

  return true;
}

const char **mitk::AdaptiveRegionGrowingTool::GetXPM() const
{
  return nullptr;
}

const char *mitk::AdaptiveRegionGrowingTool::GetName() const
{
  return "Region Growing 3D";
}

us::ModuleResource mitk::AdaptiveRegionGrowingTool::GetIconResource() const
{
  us::Module *module = us::GetModuleContext()->GetModule();
  us::ModuleResource resource = module->GetResource("RegionGrowing_48x48.png");
  return resource;
}

void mitk::AdaptiveRegionGrowingTool::Activated()
{
  Superclass::Activated();

  if (!GetDataStorage()->Exists(m_PointSetNode))
    GetDataStorage()->Add(m_PointSetNode, GetWorkingData());
  m_SeedPointInteractor = mitk::SinglePointDataInteractor::New();
  m_SeedPointInteractor->LoadStateMachine("PointSet.xml");
  m_SeedPointInteractor->SetEventConfig("PointSetConfig.xml");
  m_SeedPointInteractor->SetDataNode(m_PointSetNode);
}

void mitk::AdaptiveRegionGrowingTool::Deactivated()
{
  m_PointSet->Clear();
  GetDataStorage()->Remove(m_PointSetNode);

  Superclass::Deactivated();
}

void mitk::AdaptiveRegionGrowingTool::ConfirmSegmentation()
{
  m_ToolManager->ActivateTool(-1);
}

mitk::DataNode *mitk::AdaptiveRegionGrowingTool::GetReferenceData()
{
  return this->m_ToolManager->GetReferenceData(0);
}

mitk::DataStorage *mitk::AdaptiveRegionGrowingTool::GetDataStorage()
{
  return this->m_ToolManager->GetDataStorage();
}

mitk::DataNode *mitk::AdaptiveRegionGrowingTool::GetWorkingData()
{
  return this->m_ToolManager->GetWorkingData(0);
}

mitk::DataNode::Pointer mitk::AdaptiveRegionGrowingTool::GetPointSetNode()
{
  return m_PointSetNode;
}
