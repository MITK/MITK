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
    class MITK_CORE_EXPORT NodePredicateOR : public NodePredicateCompositeBase
    {
    public:
      mitkClassMacro(NodePredicateOR, NodePredicateCompositeBase);
      itkFactorylessNewMacro(NodePredicateOR);
      mitkNewMacro2Param(NodePredicateOR, const NodePredicateBase*, const NodePredicateBase*);

      //##Documentation
      //## @brief Standard Destructor
      virtual ~NodePredicateOR();

      //##Documentation
      //## @brief Checks, if the node fulfills any of the subpredicates conditions
      virtual bool CheckNode(const DataTreeNode* node) const;
    protected:
      //##Documentation
      //## @brief Constructor
      NodePredicateOR();
      //##Documentation
      //## @brief Convenience constructor that adds p1 and p2 to list of child predicates
      NodePredicateOR(const NodePredicateBase* p1, const NodePredicateBase* p2);
    };

} // namespace mitk

#endif /* MITKNODEPREDICATEOR_H_HEADER_INCLUDED_ */

