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


#ifndef MITKNODEPREDICATEBASE_H_HEADER_INCLUDED_
#define MITKNODEPREDICATEBASE_H_HEADER_INCLUDED_


namespace mitk {
  class DataTreeNode;
  //##Documentation
  //## @brief Interface for evaluation conditions used in the DataStorage class GetSubset() method
  //##
  //## Classes that inherit this interface can be used as predicates in the GetSubset() method
  //## of mitk::DataStorage. By combining different predicate objects, the user can form complex
  //## queries like "give me all nodes that either contain a surface object or a binary segmentation 
  //## and that are tagged as Organtype == 'Liver'".
  //##
  //## @ingroup DataStorage
  class NodePredicateBase
  {

  public:
    //##Documentation
    //## @brief Standard Constructor
    NodePredicateBase();
    //##Documentation
    //## @brief Standard Destructor
    virtual ~NodePredicateBase();

    //##Documentation
    //## @brief This method will be used to evaluate the node. Has to be overwritten in subclasses
    virtual bool CheckNode(const mitk::DataTreeNode* node) const = 0;
  };


} // namespace mitk

#endif /* MITKNODEPREDICATEBASE_H_HEADER_INCLUDED_ */
