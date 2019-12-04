/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkNodePredicateFunction.h>

mitk::NodePredicateFunction::NodePredicateFunction(const FunctionType &function)
  : m_Function(function)
{
}

mitk::NodePredicateFunction::~NodePredicateFunction()
{
}

bool mitk::NodePredicateFunction::CheckNode(const mitk::DataNode *node) const
{
  return m_Function(node);
}
