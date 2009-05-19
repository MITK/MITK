/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

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
  DataTreeNode::Pointer groupNode;
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

  DataTreeNode::Pointer groupNode;
  GetPointerParameter("Group node", groupNode);

  return image.IsNotNull() && groupNode.IsNotNull();
}
   
bool SegmentationSink::ThreadedUpdateFunction()
{
  return true;
}

/// to be called by subclasses when they want to insert some resulting object (binary image, surface, ...) into the data tree
void SegmentationSink::InsertBelowGroupNode(mitk::DataTreeNode* node)
{
  DataTreeNode* groupNode = GetGroupNode();

  if (m_DataStorage.IsNotNull())
  {
    m_DataStorage->Add( node, groupNode );
  }
    
  RenderingManager::GetInstance()->RequestUpdateAll();
}

DataTreeNode* SegmentationSink::GetGroupNode()
{
  DataTreeNode::Pointer groupNode;
  GetPointerParameter("Group node", groupNode);

  return groupNode.GetPointer();
}
    
DataTreeNode* SegmentationSink::LookForPointerTargetBelowGroupNode(const char* name)
{
  DataTreeNode::Pointer groupNode;
  GetPointerParameter("Group node", groupNode);

  if (groupNode.IsNotNull() && m_DataStorage.IsNotNull())
  {
    return m_DataStorage->GetNamedDerivedNode(name, groupNode, true);
  }

  return NULL;

}

} // namespace

