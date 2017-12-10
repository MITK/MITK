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

#include <mitkIIdentifiable.h>
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
    mitkNewMacro1Param(NodePredicateUID, const IIdentifiable::UIDType &)

    virtual ~NodePredicateUID();

    virtual bool CheckNode(const mitk::DataNode *node) const override;

  protected:
    explicit NodePredicateUID(const IIdentifiable::UIDType &uid);

    IIdentifiable::UIDType m_UID;
  };
}

#endif
