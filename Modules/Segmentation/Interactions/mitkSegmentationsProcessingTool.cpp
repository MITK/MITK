/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkSegmentationsProcessingTool.h"
#include "mitkProgressBar.h"
#include "mitkToolManager.h"

mitk::SegmentationsProcessingTool::SegmentationsProcessingTool() : Tool("dummy")
{
}

mitk::SegmentationsProcessingTool::~SegmentationsProcessingTool()
{
}

const char *mitk::SegmentationsProcessingTool::GetGroup() const
{
  return "segmentationProcessing";
}

void mitk::SegmentationsProcessingTool::Activated()
{
  Superclass::Activated();

  ProcessAllObjects();
  this->GetToolManager()->ActivateTool(-1);
}

void mitk::SegmentationsProcessingTool::Deactivated()
{
  Superclass::Deactivated();
}

void mitk::SegmentationsProcessingTool::ProcessAllObjects()
{
  m_FailedNodes.clear();
  StartProcessingAllData();

  ToolManager::DataVectorType nodes = this->GetToolManager()->GetWorkingData();
  ProgressBar::GetInstance()->AddStepsToDo(nodes.size() + 2);

  // for all selected nodes
  for (auto nodeiter = nodes.begin(); nodeiter != nodes.end(); ++nodeiter)
  {
    DataNode::Pointer node = *nodeiter;

    if (!ProcessOneWorkingData(node))
    {
      std::string nodeName;
      m_FailedNodes += " '";
      if (node->GetName(nodeName))
      {
        m_FailedNodes += nodeName.c_str();
      }
      else
      {
        m_FailedNodes += "(no name)";
      }
      m_FailedNodes += "'";
    }

    ProgressBar::GetInstance()->Progress();
  }

  FinishProcessingAllData();
  ProgressBar::GetInstance()->Progress(2);
}

void mitk::SegmentationsProcessingTool::StartProcessingAllData()
{
}

bool mitk::SegmentationsProcessingTool::ProcessOneWorkingData(DataNode *)
{
  return true;
}

void mitk::SegmentationsProcessingTool::FinishProcessingAllData()
{
  SendErrorMessageIfAny();
}

void mitk::SegmentationsProcessingTool::SendErrorMessageIfAny()
{
  if (!m_FailedNodes.empty())
  {
    Tool::ErrorMessage(GetErrorMessage() + m_FailedNodes);
  }
}

std::string mitk::SegmentationsProcessingTool::GetErrorMessage()
{
  return "Processing of these nodes failed:";
}
