/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
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


#ifndef MITKDATASTORAGE_H_HEADER_INCLUDED_
#define MITKDATASTORAGE_H_HEADER_INCLUDED_

#include <itkObject.h>
#include <mitkCommon.h>
#include <mitkDataTree.h>

#include <itkVectorContainer.h>

namespace mitk {

  class NodePredicateBase;
  class DataTreeNode;
  

  //##Documentation
  //## @brief Data management class that handles 'was created by' relations
  //##
  //## The DataStorage provides data storage and management functionality.
  //## It handles a 'was created by' relation by associating each data object with a 
  //## set of source objects, that this object was created from. The DataStorage
  //## class provides an observer mechanism to notify special agents of changes in the data
  //## object that it manages. This can be used for example to automatically update an object
  //## everytime one of its source objects changes (eg. update a surface created from 
  //## a segmentation.
  //## 
  //## @ingroup DataStorage
  class DataStorage : public itk::Object
  {
  public:
    mitkClassMacro(DataStorage, itk::Object);
    itkNewMacro(Self);

    //##Documentation
    //## @brief A Container of objects that is used as a resultset of GetSubset() query operations (Set of SmartPointers to DataTreeNodes).
    typedef itk::VectorContainer<unsigned int, mitk::DataTreeNode::Pointer> SetOfObjects;

    //##Documentation
    //## @brief Adds a DataTreeNode containing a data object to its internal storage
    //##
    //## This Method adds a new data object to the DataStorage. The new object is 
    //## passed in the first parameter. The second parameter is a set 
    //## of source objects, that were used to create this object. The new object will have
    //## a 'was created from' relation to its source objects.
    //## the addition of a new object will fire the notification mechanism.
    //## If the node parameter is NULL or if the DataTreeNode has already been added, 
    //## an exception will be thrown.
    void Add(mitk::DataTreeNode* node, const mitk::DataStorage::SetOfObjects* parents = NULL);

    //##Documentation
    //## @brief Notifies the DataStorage that the object in node has been modified
    //##
    //## Calling the Update() method will start the nofitication mechanism that will notifiy 
    //## all registered agents that the object has been updated. Depending on the registered
    //## agents, this could lead to an update of other objects, like surfaces that were created
    //## from segmentations.
    //## If the node parameter is NULL or if the DataTreeNode is not contained in the 
    //## DataStorage, an exception will be thrown.
    void Update(mitk::DataTreeNode* node);

    //##Documentation
    //## @brief returns a set of data objects that meet the given condition(s)
    //##
    //## GetSubset returns a set of objects  with a specific datatype that meet the condition(s) 
    //## specified in the condition parameter. Conditions can be 
    //##  - data type of the data object
    //##  - is source object of specific object (e.g. all source objects of node x)
    //##  - has property with specific value (e.g. OrganType is Liver)
    //##  - negation of any condition
    //##  - conjunction of a set of conditions
    //##  - disjunction of a set of conditions
    //## Conditions are implemented as predicates using the Composite Design Pattern
    //## (see definition of NodePredicateBase for details).
    //## The method returns a set of SmartPointers to the DataTreeNodes that fulfill the 
    //## conditions. A set of all objects can be retrieved with the GetAll() method;
    SetOfObjects::ConstPointer GetSubset(const NodePredicateBase& condition);
    
    //##Documentation
    //## @brief returns a set of all dataobjects that are stored in the data storage
    //##
    SetOfObjects::ConstPointer GetAll();

    //##Documentation
    //## @brief Initializes the class by providing the data tree that should be used for data storage
    //##
    void Initialize(mitk::DataTree* tree);

  protected:
    //##Documentation
    //## @brief Standard Constructor for ::New() instantiation     
    DataStorage();
    //##Documentation
    //## @brief Standard Destructor
    virtual ~DataStorage();

    //##Documentation
    //## @brief convenience method to check if the object has been initialized (i.e. a data tree has been set)
    bool IsInitialized();

    //mitk::DataTreeIteratorBase& GetElementsIterator();

    //##Documentation
    //## @brief holds the data tree that is encapsulated by this class
    mitk::DataTree::Pointer m_DataTree;
  };


} // namespace mitk

#endif /* MITKDATASTORAGE_H_HEADER_INCLUDED_ */
