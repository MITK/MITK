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


#ifndef MITKNODEPREDICATEFIRSTLEVEL_H_HEADER_INCLUDED_
#define MITKNODEPREDICATEFIRSTLEVEL_H_HEADER_INCLUDED_

#include "mitkNodePredicateBase.h"
#include "mitkDataStorage.h"
#include "mitkDataNode.h"
#include "mitkWeakPointer.h"

namespace mitk {

    //##Documentation
    //## @brief Predicate that evaluates if the given node is a direct or indirect source node of a specific node
    //##
    //## @warning This class seems to be obsolete since mitk::DataStorage::GetDerivations().
    //##          Since there is no real use case up until now, NodePredicateSource is NOT WORKING YET.
    //##          If you need it working, inform us.
    //##
    //## @ingroup DataStorage
    class MITK_CORE_EXPORT NodePredicateFirstLevel : public NodePredicateBase
    {
    public:
      mitkClassMacro(NodePredicateFirstLevel, NodePredicateBase);
      mitkNewMacro1Param(NodePredicateFirstLevel, mitk::DataStorage*);

      //##Documentation
      //## @brief Standard Destructor
      virtual ~NodePredicateFirstLevel();

      //##Documentation
      //## @brief Checks, if the node is a source node of m_BaseNode (e.g. if m_BaseNode "was created from" node)
      virtual bool CheckNode(const mitk::DataNode* node) const;

    protected:
      //##Documentation
      //## @brief Constructor - This class can either search only for direct source objects or for all source objects
      NodePredicateFirstLevel(mitk::DataStorage* ds);

      mitk::WeakPointer<mitk::DataStorage> m_DataStorage;
    };

} // namespace mitk

#endif /* MITKNODEPREDICATEFIRSTLEVEL_H_HEADER_INCLUDED_ */
