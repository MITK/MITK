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


#ifndef MITKNODEPREDICATEDATATYPE_H_HEADER_INCLUDED_
#define MITKNODEPREDICATEDATATYPE_H_HEADER_INCLUDED_

#include "mitkNodePredicateBase.h"
#include "mitkDataNode.h"
#include <string>

namespace mitk {

  //##Documentation
  //## @brief Predicate that evaluates if the given DataNodes data object is of a specific data type
  //##
  //## The data type must be specified in the constructor as a string. The string must equal the result
  //## value of the requested data types GetNameOfClass() method.
  //##
  //## @ingroup DataStorage
  class MITK_CORE_EXPORT NodePredicateDataType : public NodePredicateBase
  {
  public:
    mitkClassMacro(NodePredicateDataType, NodePredicateBase);
    mitkNewMacro1Param(NodePredicateDataType, const char*);

    //##Documentation
    //## @brief Standard Destructor
    virtual ~NodePredicateDataType();

    //##Documentation
    //## @brief Checks, if the nodes data object is of a specific data type
    virtual bool CheckNode(const mitk::DataNode* node) const;

  protected:
    //##Documentation
    //## @brief Protected constructor, use static instantiation functions instead
    NodePredicateDataType(const char* datatype);

    std::string m_ValidDataType;
  };

  /**
   * \brief Tests for type compatibility (dynamic_cast).
   *
   * In contrast to NodePredicateDataType this class also accepts derived types.
   * E.g. if you query for type BaseData, you will also get Image and Surface objects.
   *
   * The desired type is given as a template parameter, the constructor takes no other parameters.
   */
  template <class T>
  class TNodePredicateDataType : public NodePredicateBase
  {
  public:
    mitkClassMacro(TNodePredicateDataType, NodePredicateBase);
    itkFactorylessNewMacro(TNodePredicateDataType);

    virtual ~TNodePredicateDataType()
    {
    }

    //##Documentation
    //## @brief Checks, if the nodes data object is of a specific data type (casts)
    virtual bool CheckNode(const mitk::DataNode* node) const
    {
      return node && node->GetData() && dynamic_cast<T*>(node->GetData());
    }
  protected:
    //##Documentation
    //## @brief Protected constructor, use static instantiation functions instead
    TNodePredicateDataType()
    {
    }
  };

} // namespace mitk

#endif /* MITKNODEPREDICATEDATATYPE_H_HEADER_INCLUDED_ */

