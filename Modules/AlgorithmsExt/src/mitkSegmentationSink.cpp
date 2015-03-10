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

#include "mitkSegmentationSink.h"
#include "mitkRenderingManager.h"
#include "mitkDataStorage.h"

namespace mitk {

SegmentationSink::SegmentationSink()
{
}

SegmentationSink::~SegmentationSink()
{
}

void SegmentationSink::Initialize(const NonBlockingAlgorithm* other)
{
  Superclass::Initialize(other);
  // sinks should be called explicitly from the tool, because otherwise the order of setting "Input" and "Group node" would matter
  UnDefineTriggerParameter("Input");

  // some basedata output
  DataNode::Pointer groupNode;
  bool showResult(true);

  if (other)
  {
    other->GetPointerParameter("Group node", groupNode);
    other->GetParameter("Show result", showResult);
  }

  SetPointerParameter("Group node", groupNode );
  SetParameter("Show result", showResult );
}

bool SegmentationSink::ReadyToRun()
{
  Image::Pointer image;
  GetPointerParameter("Input", image);

  DataNode::Pointer groupNode;
  GetPointerParameter("Group node", groupNode);

  return image.IsNotNull() && groupNode.IsNotNull();
}

bool SegmentationSink::ThreadedUpdateFunction()
{
  return true;
}

/// to be called by subclasses when they want to insert some resulting object (binary image, surface, ...) into the data tree
void SegmentationSink::InsertBelowGroupNode(mitk::DataNode* node)
{
  DataNode* groupNode = GetGroupNode();

  if (m_DataStorage.IsNotNull())
  {
    if (node)
      node->GetData()->DisconnectPipeline();
    m_DataStorage->Add( node, groupNode );
  }

  RenderingManager::GetInstance()->RequestUpdateAll();
}

DataNode* SegmentationSink::GetGroupNode()
{
  DataNode::Pointer groupNode;
  GetPointerParameter("Group node", groupNode);

  return groupNode.GetPointer();
}

DataNode* SegmentationSink::LookForPointerTargetBelowGroupNode(const char* name)
{
  DataNode::Pointer groupNode;
  GetPointerParameter("Group node", groupNode);

  if (groupNode.IsNotNull() && m_DataStorage.IsNotNull())
  {
    return m_DataStorage->GetNamedDerivedNode(name, groupNode, true);
  }

  return NULL;

}

} // namespace

