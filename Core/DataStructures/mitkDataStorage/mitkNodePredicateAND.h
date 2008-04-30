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


#ifndef MITKNODEPREDICATEAND_H_HEADER_INCLUDED_
#define MITKNODEPREDICATEAND_H_HEADER_INCLUDED_

#include "mitkNodePredicateCompositeBase.h"

namespace mitk {

    //##Documentation
    //## @brief Composite predicate that forms a logical AND relation from its child predicates
    //##
    //## 
    //## 
    //## 
    //## @ingroup DataStorage
    class MITK_CORE_EXPORT NodePredicateAND : public NodePredicateCompositeBase
    {
    public:
      //##Documentation
      //## @brief Standard constructor
      NodePredicateAND();
      //##Documentation
      //## @brief Standard Destructor
      virtual ~NodePredicateAND();
      //##Documentation
      //## @brief Convenience constructor that adds p1 and p2 to list of child predicates
      NodePredicateAND(const NodePredicateBase& p1, const NodePredicateBase& p2);

      //##Documentation
      //## @brief Checks, if the node fulfills all of the subpredicates conditions
      virtual bool CheckNode(const DataTreeNode* node) const;
    };

} // namespace mitk

#endif /* MITKNODEPREDICATEAND_H_HEADER_INCLUDED_ */

