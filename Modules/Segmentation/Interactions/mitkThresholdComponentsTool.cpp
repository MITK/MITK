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

#include "mitkThresholdComponentsTool.h"
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
  MITK_TOOL_MACRO(MITKSEGMENTATION_EXPORT, ThresholdComponentsTool, "ThresholdComponentsTool");
}

mitk::ThresholdComponentsTool::ThresholdComponentsTool()
{
  m_PointSetNode = mitk::DataNode::New();
  m_PointSetNode->GetPropertyList()->SetProperty("name", mitk::StringProperty::New("ThresholdComponents_Seedpoint"));
  m_PointSetNode->GetPropertyList()->SetProperty("helper object", mitk::BoolProperty::New(true));
  m_PointSet = mitk::PointSet::New();
  m_PointSetNode->SetData(m_PointSet);
}

mitk::ThresholdComponentsTool::~ThresholdComponentsTool()
{
}

bool mitk::ThresholdComponentsTool::CanHandle(BaseData *referenceData) const
{
  if (referenceData == NULL)
    return false;

  Image *image = dynamic_cast<Image *>(referenceData);

  if (image == NULL)
    return false;

  if (image->GetDimension() < 3)
    return false;

  return true;
}

const char **mitk::ThresholdComponentsTool::GetXPM() const
{
  return NULL;
}

const char *mitk::ThresholdComponentsTool::GetName() const
{
  return "Threshold Components";
}

us::ModuleResource mitk::ThresholdComponentsTool::GetIconResource() const
{
  us::Module *module = us::GetModuleContext()->GetModule();
  us::ModuleResource resource = module->GetResource("ThresholdComponents_48x48.png");
  return resource;
}
	
void mitk::ThresholdComponentsTool::Activated()
{
  Superclass::Activated();

  if (!GetDataStorage()->Exists(m_PointSetNode))
    GetDataStorage()->Add(m_PointSetNode, GetWorkingData());
  m_SeedPointInteractor = mitk::PointSetDataInteractor::New();
  m_SeedPointInteractor->LoadStateMachine("PointSet.xml");
  m_SeedPointInteractor->SetEventConfig("PointSetConfig.xml");
  m_SeedPointInteractor->SetDataNode(m_PointSetNode);
}

void mitk::ThresholdComponentsTool::Deactivated()
{
  m_PointSet->Clear();
  GetDataStorage()->Remove(m_PointSetNode);

  Superclass::Deactivated();
}

void mitk::ThresholdComponentsTool::ConfirmSegmentation()
{
  m_ToolManager->ActivateTool(-1);
}

mitk::DataNode *mitk::ThresholdComponentsTool::GetReferenceData()
{
  return this->m_ToolManager->GetReferenceData(0);
}

mitk::DataStorage *mitk::ThresholdComponentsTool::GetDataStorage()
{
  return this->m_ToolManager->GetDataStorage();
}

mitk::DataNode *mitk::ThresholdComponentsTool::GetWorkingData()
{
  return this->m_ToolManager->GetWorkingData(0);
}

mitk::DataNode::Pointer mitk::ThresholdComponentsTool::GetPointSetNode()
{
  return m_PointSetNode;
}
