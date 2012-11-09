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


#ifndef MITKNODEPREDICATENOT_H_HEADER_INCLUDED_
#define MITKNODEPREDICATENOT_H_HEADER_INCLUDED_

#include "mitkNodePredicateCompositeBase.h"

namespace mitk {

  //##Documentation
  //## @brief Composite predicate that negates its child predicate
  //## Changed: NodePredicateNot now derives from NodePredicateCompositeBase though it really holds
  //## only one subpredicate at any time. But logically any Predicate that has one or more subpredicate
  //## is a CompositePredicate.
  //##
  //## @ingroup DataStorage
  class MITK_CORE_EXPORT NodePredicateNot : public mitk::NodePredicateCompositeBase
  {
  public:
    mitkClassMacro(NodePredicateNot, NodePredicateCompositeBase);
    mitkNewMacro1Param(NodePredicateNot, const mitk::NodePredicateBase*);

    //##Documentation
    //## @brief Standard Destructor
    virtual ~NodePredicateNot();

    //##Documentation
    //## @brief Reimplemented, only one child predicate is allowed for the NOT predicate.
    virtual void AddPredicate(const mitk::NodePredicateBase* p);

    //##Documentation
    //## @brief Checks, if the node does not fulfill the child predicate condition
    virtual bool CheckNode(const mitk::DataNode* node) const;
  protected:
    //##Documentation
    //## @brief Constructor
    NodePredicateNot(const mitk::NodePredicateBase* p);

  };

} // namespace mitk

#endif /* MITKNODEPREDICATENOT_H_HEADER_INCLUDED_ */
