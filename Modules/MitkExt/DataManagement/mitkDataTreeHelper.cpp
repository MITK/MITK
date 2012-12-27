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

#include "mitkDataTreeHelper.h"

mitk::DataTreeIteratorClone mitk::DataTreeHelper::FindIteratorToNode(
  mitk::DataTreeBase* tree,
  const mitk::DataNode* node)
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
  const mitk::DataNode* node
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
    mitk::DataNode* node = childIterator.Get();
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
