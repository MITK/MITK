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

#ifndef mitkNodePredicateUID_h
#define mitkNodePredicateUID_h

#include <mitkIdentifiable.h>
#include <mitkNodePredicateBase.h>
#include <MitkCoreExports.h>

namespace mitk
{
  /**
  * \brief Predicate that compares the UID of the given DataNode to a specific UID.
  *
  * \ingroup DataStorage
  */
  class MITKCORE_EXPORT NodePredicateUID : public NodePredicateBase
  {
  public:
    mitkClassMacro(NodePredicateUID, NodePredicateBase)
    mitkNewMacro1Param(NodePredicateUID, const Identifiable::UIDType &)

    ~NodePredicateUID() override;

    bool CheckNode(const mitk::DataNode *node) const override;

  protected:
    explicit NodePredicateUID(const Identifiable::UIDType &uid);

    Identifiable::UIDType m_UID;
  };
}

#endif
