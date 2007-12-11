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


#ifndef MITKNODEPREDICATEDIMENSION_H_HEADER_INCLUDED_
#define MITKNODEPREDICATEDIMENSION_H_HEADER_INCLUDED_

#include "mitkNodePredicateBase.h"

namespace mitk {
 class BaseData;

  //##Documentation
  //## @brief Predicate that evaluates if the given DataTreeNodes data object 
  //## has the specified dimension, for datasets where dimension is 
  //## applicable.
  //##
  //## Evaluates to "false" for unsupported datasets.
  //## @ingroup DataStorage
  class NodePredicateDimension : public NodePredicateBase
  {
  public:
    //##Documentation
    //## @brief Standard Constructor
    NodePredicateDimension(unsigned int dimension);
    //##Documentation
    //## @brief Standard Destructor
    virtual ~NodePredicateDimension();

    //##Documentation
    //## @brief Checks if the nodes data object is of the specified dimension
    virtual bool CheckNode(const mitk::DataTreeNode* node) const;

  protected:
    unsigned int m_Dimension;
  };
} // namespace mitk

#endif /* MITKNodePredicateDimension_H_HEADER_INCLUDED_ */
