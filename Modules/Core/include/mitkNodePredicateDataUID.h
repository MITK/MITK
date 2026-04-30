/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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

    /** \brief Standard Destructor. */
    ~NodePredicateDataUID() override;

    /** \brief Checks if the node's data object has the specified UID.
     *
     * Returns false if the node or its data object is nullptr.
     *
     * \param node The DataNode to check.
     * \return True if the data object's UID matches the stored UID.
     */
    bool CheckNode(const mitk::DataNode *node) const override;

  protected:
    /** \brief Protected constructor, use static instantiation functions instead.
     *
     * \param uid The unique identifier to compare against.
     */
    explicit NodePredicateDataUID(const Identifiable::UIDType &uid);

    Identifiable::UIDType m_UID;
  };
}

#endif
