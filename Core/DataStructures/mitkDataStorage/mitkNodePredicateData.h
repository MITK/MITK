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


#ifndef MITKNODEPREDICATEDATA_H_HEADER_INCLUDED_
#define MITKNODEPREDICATEDATA_H_HEADER_INCLUDED_

#include "mitkNodePredicateBase.h"

namespace mitk {
 class BaseData;

  //##Documentation
  //## @brief Predicate that evaluates if the given DataTreeNodes data object pointer equals a given pointer
  //##
  //## NodePredicateData(NULL) returns true if a DataTreeNode does not have a data object (e.g. ->GetData() returns NULL).
  //## This could return an unexpected number of nodes (e.g. the root node of the tree)
  //## @ingroup DataStorage
  class MITK_CORE_EXPORT NodePredicateData : public NodePredicateBase
  {
  public:
    //##Documentation
    //## @brief Standard Constructor
    NodePredicateData(mitk::BaseData* d);
    //##Documentation
    //## @brief Standard Destructor
    virtual ~NodePredicateData();

    //##Documentation
    //## @brief Checks, if the nodes data object is of a specific data type 
    virtual bool CheckNode(const mitk::DataTreeNode* node) const;

  protected:
    mitk::BaseData* m_DataObject;
  };
} // namespace mitk

#endif /* MITKNODEPREDICATEDATA_H_HEADER_INCLUDED_ */
