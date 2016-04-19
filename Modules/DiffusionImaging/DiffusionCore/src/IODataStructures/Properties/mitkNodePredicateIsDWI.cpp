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

#include "mitkNodePredicateIsDWI.h"
#include "mitkDataNode.h"
#include <mitkDiffusionPropertyHelper.h>

mitk::NodePredicateIsDWI::NodePredicateIsDWI()
: NodePredicateBase()
{
}

mitk::NodePredicateIsDWI::~NodePredicateIsDWI()
{
}


bool mitk::NodePredicateIsDWI::CheckNode(const mitk::DataNode* node) const
{
  if (node == nullptr)
    throw std::invalid_argument("NodePredicateIsDWI: invalid node");

  mitk::Image* image = dynamic_cast<mitk::Image*>(node->GetData());

  return mitk::DiffusionPropertyHelper::IsDiffusionWeightedImage( image );
}
