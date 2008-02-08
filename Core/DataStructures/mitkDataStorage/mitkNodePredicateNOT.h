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

#include "mitkNodePredicateBase.h"

namespace mitk {

  //##Documentation
  //## @brief Composite predicate that negates its child predicate
  //##
  //## 
  //## 
  //## 
  //## @ingroup DataStorage
  class MITK_CORE_EXPORT NodePredicateNOT : public NodePredicateBase
  {
  public:
    //##Documentation
    //## @brief Constructor
    NodePredicateNOT(mitk::NodePredicateBase& p);
    //##Documentation
    //## @brief Standard Destructor
    virtual ~NodePredicateNOT();



    //##Documentation
    //## @brief Checks, if the node does not fulfill the child predicate condition
    virtual bool CheckNode(const mitk::DataTreeNode* node) const;


  protected:
    mitk::NodePredicateBase& m_ChildPredicate;
  };

} // namespace mitk

#endif /* MITKNODEPREDICATENOT_H_HEADER_INCLUDED_ */
