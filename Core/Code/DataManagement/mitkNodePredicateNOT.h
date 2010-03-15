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


#ifndef MITKNODEPREDICATENOT_H_HEADER_INCLUDED_
#define MITKNODEPREDICATENOT_H_HEADER_INCLUDED_

#include "mitkNodePredicateCompositeBase.h"

namespace mitk {

  //##Documentation
  //## @brief Composite predicate that negates its child predicate
  //## Changed: NodePredicateNOT now derives from NodePredicateCompositeBase though it really holds
  //## only one subpredicate at any time. But logically any Predicate that has one or more subpredicate
  //## is a CompositePredicate.
  //## 
  //## @ingroup DataStorage
  class MITK_CORE_EXPORT NodePredicateNOT : public mitk::NodePredicateCompositeBase
  {
  public:
    mitkClassMacro(NodePredicateNOT, NodePredicateCompositeBase);
    mitkNewMacro1Param(NodePredicateNOT, const mitk::NodePredicateBase*);

    //##Documentation
    //## @brief Standard Destructor
    virtual ~NodePredicateNOT();

    //##Documentation
    //## @brief Reimplemented, only one child predicate is allowed for the NOT predicate.
    virtual void AddPredicate(const mitk::NodePredicateBase* p);

    //##Documentation
    //## @brief Checks, if the node does not fulfill the child predicate condition
    virtual bool CheckNode(const mitk::DataNode* node) const;
  protected:
    //##Documentation
    //## @brief Constructor
    NodePredicateNOT(const mitk::NodePredicateBase* p);

  };

} // namespace mitk

#endif /* MITKNODEPREDICATENOT_H_HEADER_INCLUDED_ */
