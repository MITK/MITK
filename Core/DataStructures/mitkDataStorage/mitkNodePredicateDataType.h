/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#ifndef MITKNODEPREDICATEDATATYPE_H_HEADER_INCLUDED_
#define MITKNODEPREDICATEDATATYPE_H_HEADER_INCLUDED_

#include "mitkNodePredicateBase.h"
#include <string>

namespace mitk {

  //##Documentation
  //## @brief Predicate that evaluates if the given DataTreeNodes data object is of a specific data type
  //##
  //## The data type must be specified in the constructor as a string. The string must equal the result 
  //## value of the requested data types GetNameOfClass() method.
  //## 
  //## @ingroup DataStorage
  class NodePredicateDataType : public NodePredicateBase
  {
  public:
    //##Documentation
    //## @brief Standard Constructor
    NodePredicateDataType(const char* datatype);
    //##Documentation
    //## @brief Standard Destructor
    virtual ~NodePredicateDataType();

    //##Documentation
    //## @brief Checks, if the nodes data object is of a specific data type 
    virtual bool CheckNode(const mitk::DataTreeNode* node) const;

  protected:
    std::string m_ValidDataType;
  };
} // namespace mitk

#endif /* MITKNODEPREDICATEDATATYPE_H_HEADER_INCLUDED_ */
