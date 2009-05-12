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
  //## @warning NodePredicateData holds a weak pointer to a BaseData! NodePredicateData p(mitk::BaseData::New()); will not work.
  //##          Intended use is: NodePredicateData p(myDataObject); result = myDataStorage->GetSubset(p); Then work with result, do not reuse p later.
  //##          
  //## @ingroup DataStorage
  class MITK_CORE_EXPORT NodePredicateData : public NodePredicateBase
  {
  public:
    mitkClassMacro(NodePredicateData, NodePredicateBase);
    mitkNewMacro1Param(NodePredicateData, mitk::BaseData*);

    //##Documentation
    //## @brief Standard Destructor
    virtual ~NodePredicateData();

    //##Documentation
    //## @brief Checks, if the nodes data object is of a specific data type 
    virtual bool CheckNode(const mitk::DataTreeNode* node) const;

  protected:
    //##Documentation
    //## @brief Protected constructor, use static instantiation functions instead
    NodePredicateData(mitk::BaseData* d);

    mitk::BaseData* m_DataObject;
  };
} // namespace mitk

#endif /* MITKNODEPREDICATEDATA_H_HEADER_INCLUDED_ */
