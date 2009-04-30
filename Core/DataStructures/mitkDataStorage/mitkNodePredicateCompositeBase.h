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


#ifndef MITKNODEPREDICATECOMPOSITEBASE_H_HEADER_INCLUDED_
#define MITKNODEPREDICATECOMPOSITEBASE_H_HEADER_INCLUDED_

#include "mitkNodePredicateBase.h"
#include <list>

namespace mitk {

  //##Documentation
  //## @brief Base class for all predicates that can have child predicates (e.g. AND/OR)
  //##
  //## This class provides methods to add and remove child predicates. It is used for 
  //## predicates that are compositions of other predicates like AND and OR.
  //## 
  //## @ingroup DataStorage
  class MITK_CORE_EXPORT NodePredicateCompositeBase : public NodePredicateBase
  {
  public:
    mitkClassMacro(NodePredicateCompositeBase, NodePredicateBase);

    typedef std::list<const NodePredicateBase*> ChildPredicates;

    //##Documentation
    //## @brief Pure virtual (but implemented) Destructor makes NodePredicateCompositeBase an abstract class
    virtual ~NodePredicateCompositeBase() = 0;

    //##Documentation
    //## @brief Adds a child predicate
    virtual void AddPredicate(const NodePredicateBase* p);

    //##Documentation
    //## @brief Removes a child predicate
    virtual void RemovePredicate(const NodePredicateBase* p);

    //##Documentation
    //## @brief Return all child predicates (immutable).
    virtual ChildPredicates GetPredicates() const;

  protected:
    //##Documentation
    //## @brief list of child predicates
    ChildPredicates m_ChildPredicates;
  };

} // namespace mitk

#endif /* MITKNODEPREDICATECOMPOSITEBASE_H_HEADER_INCLUDED_ */

