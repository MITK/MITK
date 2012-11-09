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


#ifndef MITKNODEPREDICATEOR_H_HEADER_INCLUDED_
#define MITKNODEPREDICATEOR_H_HEADER_INCLUDED_

#include "mitkNodePredicateCompositeBase.h"

namespace mitk {

    //##Documentation
    //## @brief Composite predicate that forms a logical OR relation from its child predicates
    //##
    //##
    //##
    //##
    //## @ingroup DataStorage
    class MITK_CORE_EXPORT NodePredicateOr : public NodePredicateCompositeBase
    {
    public:
      mitkClassMacro(NodePredicateOr, NodePredicateCompositeBase);
      itkFactorylessNewMacro(NodePredicateOr);
      mitkNewMacro2Param(NodePredicateOr, const NodePredicateBase*, const NodePredicateBase*);

      //##Documentation
      //## @brief Standard Destructor
      virtual ~NodePredicateOr();

      //##Documentation
      //## @brief Checks, if the node fulfills any of the subpredicates conditions
      virtual bool CheckNode(const DataNode* node) const;
    protected:
      //##Documentation
      //## @brief Constructor
      NodePredicateOr();
      //##Documentation
      //## @brief Convenience constructor that adds p1 and p2 to list of child predicates
      NodePredicateOr(const NodePredicateBase* p1, const NodePredicateBase* p2);
    };

} // namespace mitk

#endif /* MITKNODEPREDICATEOR_H_HEADER_INCLUDED_ */

