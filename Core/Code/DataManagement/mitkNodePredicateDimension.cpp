/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision: 11215 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "mitkNodePredicateDimension.h"

#include "mitkDataTreeNode.h"


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


bool mitk::NodePredicateDimension::CheckNode(const mitk::DataTreeNode* node) const
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

