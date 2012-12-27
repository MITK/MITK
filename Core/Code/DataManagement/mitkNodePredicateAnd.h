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
    class MITK_CORE_EXPORT NodePredicateAnd : public NodePredicateCompositeBase
    {
    public:
      mitkClassMacro(NodePredicateAnd, NodePredicateCompositeBase);

      itkFactorylessNewMacro(NodePredicateAnd);
      mitkNewMacro2Param(NodePredicateAnd, const NodePredicateBase*, const NodePredicateBase*);
      mitkNewMacro3Param(NodePredicateAnd, const NodePredicateBase*, const NodePredicateBase*, const NodePredicateBase*);

      //##Documentation
      //## @brief Standard Destructor
      virtual ~NodePredicateAnd();

      //##Documentation
      //## @brief Checks, if the node fulfills all of the subpredicates conditions
      virtual bool CheckNode(const DataNode* node) const;
    protected:
      //##Documentation
      //## @brief Protected constructor, use static instantiation functions instead
      NodePredicateAnd();

      //##Documentation
      //## @brief Convenience constructor that adds p1 and p2 to list of child predicates
      //## Protected constructor, use static instantiation functions instead
      NodePredicateAnd(const NodePredicateBase* p1, const NodePredicateBase* p2);

      //##Documentation
      //## @brief Convenience constructor that adds p1, p2 and p3 to list of child predicates
      //## Protected constructor, use static instantiation functions instead
      NodePredicateAnd(const NodePredicateBase* p1, const NodePredicateBase* p2, const NodePredicateBase* p3);
    };

} // namespace mitk

#endif /* MITKNODEPREDICATEAND_H_HEADER_INCLUDED_ */

