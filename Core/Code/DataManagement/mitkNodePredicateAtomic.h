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


#ifndef MITKNODEPREDICATEATOMIC_H_HEADER_INCLUDED_
#define MITKNODEPREDICATEATOMIC_H_HEADER_INCLUDED_

#include "mitkNodePredicateBase.h"

namespace mitk {

  //##Documentation
  //## @brief Atomic predicates that evaluate 'true' or 'false' for every valid node
  //##
  //## The class cannot be instantiated. You can refer to its two predefined objects by
  //## mitk::NodePredicateAtomic::FALSE and mitk::NodePredicateAtomic::TRUE, respectively.
  //##
  //## @ingroup DataStorage
  class MITK_CORE_EXPORT NodePredicateAtomic : public NodePredicateBase
  {
  public:
//    mitkClassMacro(NodePredicateAtomic, NodePredicateBase);

    //##Documentation
    //## @brief Standard Destructor
    virtual ~NodePredicateAtomic();

    //##Documentation
    //## @brief Returns 'true' or 'false' for every valid node. If the parameter is NULL, an exception is thrown.
    virtual bool CheckNode(const mitk::DataNode* node) const;

    // Smart pointers are not needed, since only these two instances will exist.
    static NodePredicateAtomic* False;
    static NodePredicateAtomic* True;

  private:
    //##Documentation
    //## @brief Protected constructor, use static instantiation functions instead
    NodePredicateAtomic(bool value);

    bool m_Value;
  };
} // namespace mitk

#endif /* MITKNODEPREDICATEATOMIC_H_HEADER_INCLUDED_ */
