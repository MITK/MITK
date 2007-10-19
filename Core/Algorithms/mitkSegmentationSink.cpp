/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
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
  DataTree::Pointer dataTree;
  bool showResult(true);

  if (other)
  {
    other->GetPointerParameter("Group node", groupNode);
    other->GetPointerParameter("Data tree", dataTree);
    other->GetParameter("Show result", showResult);
  }

  SetPointerParameter("Group node", groupNode );
  SetPointerParameter("Data tree", dataTree );
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
  DataTree::Pointer dataTree;
  GetPointerParameter("Data tree", dataTree );

  DataTreeNode* groupNode = GetGroupNode();

  DataTreeIteratorClone treeIter = dataTree->GetIteratorToNode( groupNode );

  if (treeIter->IsAtEnd()) treeIter->GoToBegin();
  
  treeIter->Add(node); // insert below the group node
    
  // TODO synchronize some properties... (?)
    // synchronize color?    optional?
    // synchronize visibility?
  RenderingManager::GetInstance()->RequestUpdateAll(true); // including vtk actors
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

  DataTree::Pointer dataTree;
  GetPointerParameter("Data tree", dataTree );

  if (groupNode.IsNotNull() && dataTree.IsNotNull())
  {
    DataTreeIteratorClone treeIter = dataTree->GetIteratorToNode( groupNode );

    SmartPointerProperty* spp = dynamic_cast<SmartPointerProperty*>( groupNode->GetProperty(name).GetPointer() );

    if (spp) // try to find this node as a child 
    {
      DataTreeNode* lookedForChildNode = dynamic_cast<DataTreeNode*>( spp->GetSmartPointer().GetPointer() );
      
      if (lookedForChildNode)
      {
        int position = treeIter->ChildPosition( lookedForChildNode );
        if ( position != -1 )
        {
          treeIter->GoToChild( position );
          return treeIter->Get();
        }
      }

    }
  }

  return NULL;

}

} // namespace

