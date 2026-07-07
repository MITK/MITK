/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkNodePredicateCompositeBase_h
#define mitkNodePredicateCompositeBase_h

#include <mitkNodePredicateBase.h>
#include <list>

namespace mitk
{
  /**
   * \brief Abstract base class for composite predicates that contain child predicates (e.g., AND, OR, NOT).
   *
   * This class provides methods to add, remove, and retrieve child predicates.
   * It serves as the foundation for predicates that combine multiple conditions,
   * such as NodePredicateAnd, NodePredicateOr, and NodePredicateNot.
   *
   * \ingroup DataStorage
   * \sa NodePredicateAnd, NodePredicateOr, NodePredicateNot, NodePredicateBase
   */
  class MITKCORE_EXPORT NodePredicateCompositeBase : public NodePredicateBase
  {
  public:
    mitkClassMacro(NodePredicateCompositeBase, NodePredicateBase);

    /** \brief List type for storing child predicate smart pointers. */
    typedef std::list<NodePredicateBase::ConstPointer> ChildPredicates;

    /**
     * \brief Pure virtual destructor (with implementation) making this class abstract.
     */
    ~NodePredicateCompositeBase() override = 0;

    /**
     * \brief Add a child predicate to this composite.
     *
     * \param[in] p  The child predicate to add. Must not be nullptr.
     * \sa RemovePredicate, GetPredicates
     */
    virtual void AddPredicate(const NodePredicateBase *p);

    /**
     * \brief Remove a child predicate from this composite.
     *
     * \param[in] p  The child predicate to remove.
     * \sa AddPredicate
     */
    virtual void RemovePredicate(const NodePredicateBase *p);

    /**
     * \brief Return all child predicates (immutable copy).
     *
     * \return A copy of the list of child predicates.
     */
    virtual ChildPredicates GetPredicates() const;

  protected:
    /** \brief List of child predicates managed by this composite. */
    ChildPredicates m_ChildPredicates;
  };

} // namespace mitk

#endif
