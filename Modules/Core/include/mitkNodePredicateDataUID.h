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

#ifndef mitkNodePredicateDataUID_h
#define mitkNodePredicateDataUID_h

#include <mitkIdentifiable.h>
#include <mitkNodePredicateBase.h>
#include <MitkCoreExports.h>

namespace mitk
{
  /**
   * \brief Predicate that compares the data UID of the given DataNode to a specific UID.
   *
   * \ingroup DataStorage
   */
  class MITKCORE_EXPORT NodePredicateDataUID : public NodePredicateBase
  {
  public:
    mitkClassMacro(NodePredicateDataUID, NodePredicateBase)
    mitkNewMacro1Param(NodePredicateDataUID, const Identifiable::UIDType &)

    ~NodePredicateDataUID() override;

    bool CheckNode(const mitk::DataNode *node) const override;

  protected:
    explicit NodePredicateDataUID(const Identifiable::UIDType &uid);

    Identifiable::UIDType m_UID;
  };
}

#endif
