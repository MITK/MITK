/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkLabelHighlightGuard.h>

#include <mitkExceptionMacro.h>
#include <mitkProperties.h>
#include <mitkRenderingManager.h>
#include <mitkVectorProperty.h>


void mitk::LabelHighlightGuard::SetSegmentationNode(DataNode* node)
{
  auto ownNode = m_Node.Lock();
  if (node != ownNode)
  {
    UpdateNode(ownNode, {}, false);
    m_Node = node;
    m_Labels.clear();
    UpdateNode(node, m_Labels, m_HighlightInvisible);
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
};

mitk::DataNode::Pointer mitk::LabelHighlightGuard::GetSegmentationNode() const
{
  return m_Node.Lock();
}

void mitk::LabelHighlightGuard::SetHighlightedLabels(MultiLabelSegmentation::LabelValueVectorType labels)
{
  auto ownNode = m_Node.Lock();
  if (ownNode.IsNotNull() && labels != m_Labels)
  {
    m_Labels = labels;
    UpdateNode(ownNode, m_Labels, m_HighlightInvisible);
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
}

void mitk::LabelHighlightGuard::SetHighlightInvisibleLabels(bool highlightInvisible)
{
  auto ownNode = m_Node.Lock();
  if (highlightInvisible == m_HighlightInvisible)
    return;

  m_HighlightInvisible = highlightInvisible;

  if (ownNode.IsNotNull() && !m_Labels.empty())
  {
    UpdateNode(ownNode, m_Labels, m_HighlightInvisible);
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
}

mitk::LabelHighlightGuard::~LabelHighlightGuard()
{
  this->SetSegmentationNode(nullptr);
}

void mitk::LabelHighlightGuard::UpdateNode(DataNode* node, MultiLabelSegmentation::LabelValueVectorType labels, bool highlightInvisible)
{
  if (nullptr == node)
    return;

  mitk::IntVectorProperty::Pointer labelProperty = dynamic_cast<mitk::IntVectorProperty*>(node->GetNonConstProperty(PROPERTY_NAME_LABELS_HIGHLIGHTED()));
  if (nullptr == labelProperty)
  {
    labelProperty = mitk::IntVectorProperty::New();
    node->SetProperty(PROPERTY_NAME_LABELS_HIGHLIGHTED(), labelProperty);
  }

  mitk::IntVectorProperty::VectorType intValues(labels.begin(), labels.end());
  labelProperty->SetValue(intValues);
  labelProperty->Modified(); //see T30386; needed because VectorProperty::SetValue does currently trigger no modified

  mitk::BoolProperty::Pointer invisibleProperty = dynamic_cast<mitk::BoolProperty*>(node->GetNonConstProperty(PROPERTY_NAME_HIGHLIGHT_INVISIBLE()));
  if (nullptr == invisibleProperty)
  {
    invisibleProperty = mitk::BoolProperty::New();
    node->SetProperty(PROPERTY_NAME_HIGHLIGHT_INVISIBLE(), invisibleProperty);
  }

  invisibleProperty->SetValue(highlightInvisible);
  invisibleProperty->Modified(); //see T30386; needed because VectorProperty::SetValue does currently trigger no modified
}
