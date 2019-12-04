/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef MITKNODEPREDICATEDATAPROPERTY_H_HEADER_INCLUDED_
#define MITKNODEPREDICATEDATAPROPERTY_H_HEADER_INCLUDED_

#include "mitkBaseProperty.h"
#include "mitkNodePredicateBase.h"

namespace mitk
{
  /** @brief Predicate that evaluates if the data of a given DataNode has a specific property.
   If the second parameter is nullptr, it will only be checked whether there is a property with the specified name
   for the data instance of the node.*/
  class MITKCORE_EXPORT NodePredicateDataProperty : public NodePredicateBase
  {
  public:
    mitkClassMacro(NodePredicateDataProperty, NodePredicateBase);
    mitkNewMacro1Param(NodePredicateDataProperty, const char *);
    mitkNewMacro2Param(NodePredicateDataProperty, const char *, mitk::BaseProperty *);

    //##Documentation
    //## @brief Standard Destructor
    ~NodePredicateDataProperty() override;

    //##Documentation
    //## @brief Checks, if the nodes contains a property that is equal to m_ValidProperty
    bool CheckNode(const mitk::DataNode *node) const override;

  protected:
    //##Documentation
    //## @brief Constructor to check for a named property
    NodePredicateDataProperty(const char *propertyName,
                          mitk::BaseProperty *p = nullptr);

    mitk::BaseProperty::Pointer m_ValidProperty;
    // mitk::BaseProperty* m_ValidProperty;
    std::string m_ValidPropertyName;
  };

} // namespace mitk

#endif /* MITKNODEPREDICATEPROPERTY_H_HEADER_INCLUDED_ */
