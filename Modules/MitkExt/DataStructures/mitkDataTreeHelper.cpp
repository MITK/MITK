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

#include "mitkDataTreeHelper.h"

mitk::DataTreeIteratorClone mitk::DataTreeHelper::FindIteratorToNode(
  mitk::DataTreeBase* tree,
  const mitk::DataTreeNode* node)
{
  if(tree == NULL)
    return NULL;

  DataTreeIteratorClone pos = DataTreePreOrderIterator(tree);

  while ( !pos->IsAtEnd() )
  {
    if ( pos->Get().GetPointer() == node )
      return pos;
    ++pos;
  }
  return pos;
}

mitk::DataTreeIteratorClone mitk::DataTreeHelper::FindIteratorToNode(
  const mitk::DataTreeIteratorBase* startPosition,
  const mitk::DataTreeNode* node
  )
{
  DataTreeIteratorClone pos = *startPosition;

  while ( !pos->IsAtEnd() )
  {
    if ( pos->Get().GetPointer() == node )
      return pos;
    ++pos;
  }
  return pos;
}

mitk::DataTreeIteratorClone mitk::DataTreeHelper::GetNamedChild(mitk::DataTreeIteratorBase* iterator, std::string name)
{
  mitk::DataTreeChildIterator childIterator(*iterator);
  while (! childIterator.IsAtEnd() )
  {
    mitk::DataTreeNode* node = childIterator.Get();
    std::string nodeName;
    if (node && node->GetName(nodeName))
    {
      if (nodeName == name)
      {
        return DataTreeIteratorClone(childIterator);
      }
    }
    ++childIterator;
  }
  return DataTreeIteratorClone(NULL);
}
