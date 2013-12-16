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

#include "mitkPickingTool.h"

#include "mitkToolManager.h"
#include "mitkProperties.h"
#include <mitkInteractionConst.h>
#include "mitkGlobalInteraction.h"

// us
#include <usModule.h>
#include <usModuleResource.h>
#include <usGetModuleContext.h>
#include <usModuleContext.h>

#include "itkAndImageFilter.h"

#include "mitkImageCast.h"
#include "mitkImageTimeSelector.h"
#include "mitkPointSetInteractor.h"
#include "mitkProperties.h"
#include "mitkITKImageImport.h"
#include "mitkImageAccessByItk.h"


namespace mitk {
  MITK_TOOL_MACRO(Segmentation_EXPORT, PickingTool, "PickingTool");
}

mitk::PickingTool::PickingTool()
{
  m_PointSetNode = mitk::DataNode::New();
  m_PointSetNode->GetPropertyList()->SetProperty("name", mitk::StringProperty::New("Picking_Seedpoint"));
  m_PointSetNode->GetPropertyList()->SetProperty("helper object", mitk::BoolProperty::New(true));
  m_PointSet = mitk::PointSet::New();
  m_PointSetNode->SetData(m_PointSet);
  m_SeedPointInteractor = mitk::PointSetInteractor::New("singlepointinteractor", m_PointSetNode);

  //Watch for point added or modified
  itk::SimpleMemberCommand<PickingTool>::Pointer pointAddedCommand = itk::SimpleMemberCommand<PickingTool>::New();
  pointAddedCommand->SetCallbackFunction(this, &PickingTool::OnPointAdded);
  m_PointSetAddObserverTag = m_PointSetNode->AddObserver( mitk::PointSetAddEvent(), pointAddedCommand);
}

mitk::PickingTool::~PickingTool()
{
}

const char** mitk::PickingTool::GetXPM() const
{
  return NULL;
}

const char* mitk::PickingTool::GetName() const
{
  return "Picking";
}

us::ModuleResource mitk::PickingTool::GetIconResource() const
{
  us::Module* module = us::GetModuleContext()->GetModule();
  us::ModuleResource resource = module->GetResource("RegionGrowing_48x48.png");
  return resource;
}

void mitk::PickingTool::Activated()
{
  if (!GetDataStorage()->Exists(m_PointSetNode))
    GetDataStorage()->Add(m_PointSetNode, GetWorkingData());
  mitk::GlobalInteraction::GetInstance()->AddInteractor(m_SeedPointInteractor);
}

void mitk::PickingTool::Deactivated()
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

mitk::DataNode* mitk::PickingTool::GetReferenceData(){
  return this->m_ToolManager->GetReferenceData(0);
}

mitk::DataStorage* mitk::PickingTool::GetDataStorage(){
  return this->m_ToolManager->GetDataStorage();
}

mitk::DataNode* mitk::PickingTool::GetWorkingData(){
  return this->m_ToolManager->GetWorkingData(0);
}

mitk::DataNode::Pointer mitk::PickingTool::GetPointSetNode()
{
  return m_PointSetNode;
}

void mitk::PickingTool::OnPointAdded()
{
  //Perform region picking
}
