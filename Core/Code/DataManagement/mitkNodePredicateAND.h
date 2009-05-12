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
      mitkClassMacro(NodePredicateAND, NodePredicateCompositeBase);

      itkFactorylessNewMacro(NodePredicateAND);
      mitkNewMacro2Param(NodePredicateAND, const NodePredicateBase*, const NodePredicateBase*);
      mitkNewMacro3Param(NodePredicateAND, const NodePredicateBase*, const NodePredicateBase*, const NodePredicateBase*);

      //##Documentation
      //## @brief Standard Destructor
      virtual ~NodePredicateAND();

      //##Documentation
      //## @brief Checks, if the node fulfills all of the subpredicates conditions
      virtual bool CheckNode(const DataTreeNode* node) const;
    protected:
      //##Documentation
      //## @brief Protected constructor, use static instantiation functions instead
      NodePredicateAND();

      //##Documentation
      //## @brief Convenience constructor that adds p1 and p2 to list of child predicates
      //## Protected constructor, use static instantiation functions instead
      NodePredicateAND(const NodePredicateBase* p1, const NodePredicateBase* p2);
  
      //##Documentation
      //## @brief Convenience constructor that adds p1, p2 and p3 to list of child predicates
      //## Protected constructor, use static instantiation functions instead
      NodePredicateAND(const NodePredicateBase* p1, const NodePredicateBase* p2, const NodePredicateBase* p3);
    };

} // namespace mitk

#endif /* MITKNODEPREDICATEAND_H_HEADER_INCLUDED_ */

