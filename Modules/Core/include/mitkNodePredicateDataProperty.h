/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkNodePredicateDataProperty_h
#define mitkNodePredicateDataProperty_h

#include <mitkBaseProperty.h>
#include <mitkNodePredicateBase.h>

namespace mitk
{
  /** \brief Predicate that evaluates if the data of a given DataNode has a specific property.
   *
   * If the second parameter is nullptr, it will only be checked whether there is a property
   * with the specified name for the data instance of the node.
   *
   * \ingroup DataStorage
   */
  class MITKCORE_EXPORT NodePredicateDataProperty : public NodePredicateBase
  {
  public:
    mitkClassMacro(NodePredicateDataProperty, NodePredicateBase);
    mitkNewMacro1Param(NodePredicateDataProperty, const char *);
    mitkNewMacro2Param(NodePredicateDataProperty, const char *, mitk::BaseProperty *);

    /** \brief Standard Destructor. */
    ~NodePredicateDataProperty() override;

    /** \brief Checks if the node's data object has a property matching the stored criteria.
     *
     * If a valid property was supplied at construction, both name and value must match.
     * If only a name was supplied, only the existence of the property is checked.
     *
     * \param node The DataNode to check.
     * \return True if the data object has the matching property.
     */
    bool CheckNode(const mitk::DataNode *node) const override;

  protected:
    /** \brief Constructor to check for a named property.
     *
     * \param propertyName Name of the property to look for (must not be empty).
     * \param p Optional property value to compare against. If nullptr, only
     *          the existence of the named property is checked.
     */
    NodePredicateDataProperty(const char *propertyName,
                          mitk::BaseProperty *p = nullptr);

    mitk::BaseProperty::Pointer m_ValidProperty;
    // mitk::BaseProperty* m_ValidProperty;
    std::string m_ValidPropertyName;
  };

} // namespace mitk

#endif
