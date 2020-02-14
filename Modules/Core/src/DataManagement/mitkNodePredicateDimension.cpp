/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkNodePredicateDimension.h"
#include "mitkDataNode.h"
#include "mitkImage.h"

mitk::NodePredicateDimension::NodePredicateDimension(unsigned int dimension, int pixelComponents)
  : m_Dimension(dimension), m_PixelComponents(pixelComponents)
{
}

mitk::NodePredicateDimension::NodePredicateDimension(unsigned int dimension)
  : m_Dimension(dimension), m_PixelComponents(1)
{
}

mitk::NodePredicateDimension::~NodePredicateDimension()
{
}

bool mitk::NodePredicateDimension::CheckNode(const mitk::DataNode *node) const
{
  if (node == nullptr)
    throw std::invalid_argument("NodePredicateDimension: invalid node");

  auto *image = dynamic_cast<mitk::Image *>(node->GetData());
  if (image != nullptr)
  {
    return (image->GetDimension() == m_Dimension && image->GetPixelType().GetNumberOfComponents() == m_PixelComponents);
  }

  return false;
}
