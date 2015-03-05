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

#include "mitkNodePredicateDimension.h"
#include "mitkImage.h"
#include "mitkDataNode.h"


mitk::NodePredicateDimension::NodePredicateDimension(unsigned int dimension, int pixelComponents)
: m_Dimension( dimension ),
  m_PixelComponents( pixelComponents )
{
}

mitk::NodePredicateDimension::NodePredicateDimension( unsigned int dimension )
: m_Dimension( dimension ),
m_PixelComponents(1)
{

}

mitk::NodePredicateDimension::~NodePredicateDimension()
{
}


bool mitk::NodePredicateDimension::CheckNode(const mitk::DataNode* node) const
{
  if (node == NULL)
    throw std::invalid_argument("NodePredicateDimension: invalid node");

  mitk::Image *image = dynamic_cast<mitk::Image *>( node->GetData() );
  if (image != NULL)
  {
    return (image->GetDimension() == m_Dimension && image->GetPixelType().GetNumberOfComponents() == m_PixelComponents);
  }

  return false;
}

