/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef MITKNODEPREDICATEFIRSTLEVEL_H_HEADER_INCLUDED_
#define MITKNODEPREDICATEFIRSTLEVEL_H_HEADER_INCLUDED_

#include "mitkDataNode.h"
#include "mitkDataStorage.h"
#include "mitkNodePredicateBase.h"
#include "mitkWeakPointer.h"

namespace mitk
{
  //##Documentation
  //## @brief Predicate that evaluates if the given node is a direct or indirect source node of a specific node
  //##
  //## @warning This class seems to be obsolete since mitk::DataStorage::GetDerivations().
  //##          Since there is no real use case up until now, NodePredicateSource is NOT WORKING YET.
  //##          If you need it working, inform us.
  //##
  //## @ingroup DataStorage
  class MITKCORE_EXPORT NodePredicateFirstLevel : public NodePredicateBase
  {
  public:
    mitkClassMacro(NodePredicateFirstLevel, NodePredicateBase);
    mitkNewMacro1Param(NodePredicateFirstLevel, mitk::DataStorage *);

    //##Documentation
    //## @brief Standard Destructor
    ~NodePredicateFirstLevel() override;

    //##Documentation
    //## @brief Checks, if the node is a source node of m_BaseNode (e.g. if m_BaseNode "was created from" node)
    bool CheckNode(const mitk::DataNode *node) const override;

  protected:
    //##Documentation
    //## @brief Constructor - This class can either search only for direct source objects or for all source objects
    NodePredicateFirstLevel(mitk::DataStorage *ds);

    mitk::WeakPointer<mitk::DataStorage> m_DataStorage;
  };

} // namespace mitk

#endif /* MITKNODEPREDICATEFIRSTLEVEL_H_HEADER_INCLUDED_ */
