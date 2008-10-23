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


#ifndef MITKDATASTORAGE_H_HEADER_INCLUDED_
#define MITKDATASTORAGE_H_HEADER_INCLUDED_

#include "itkObject.h"
#include "mitkCommon.h"
#include "mitkDataTree.h"
#include "mitkMessage.h"
#include "itkVectorContainer.h"
#include <map>

namespace mitk {

  class NodePredicateBase;
  class DataTreeNode;
  class BaseRenderer;
  class Geometry2DDataVtkMapper3D;

  //##Documentation
  //## @brief Data management class that handles 'was created by' relations
  //##
  //## The DataStorage provides data storage and management functionality.
  //## It handles a 'was created by' relation by associating each data object with a
  //## set of source objects, that this object was created from. 
  //## Thus, nodes are stored in a noncyclical directed graph data structure.
  //## If a new node is added to the DataStorage, AddNodeEvent is emitted. 
  //## If a node is removed, RemoveNodeEvent is emitted. 
  //## Currently, DataStorage uses an underlying mitk::DataTree. Therefore a DataStorage object
  //## must be initialized with a DataTree before it can be used by calling ->Initialize(myTree).
  //## @warning DataStorage methods will raise exceptions if called before the DataStorage is initialized.
  //##
  //## All objects that are added to DataStorage get automatically added to the underlying tree.
  //## The tree can be used to get notifications on New/Delete events.
  //## @warning Do not mix Adding/Removing objects with the DataStorage and DataTree methods.
  //## @warning The DataStorage does not automatically return all objects that are stored in the
  //##          DataTree. Use SetManageCompleteTree() to enable/disable management of objects that were
  //##          added to the tree directly.
  //##
  //## @ingroup DataStorage
  class MITK_CORE_EXPORT DataStorage : public itk::Object
  {
  public:
    mitkClassMacro(DataStorage, itk::Object);
    //itkNewMacro(Self);

    static DataStorage* CreateInstance(mitk::DataTree* tree);  // create method that initializes singleton object
    static DataStorage* GetInstance();    // Singleton pattern like access method

    //##Documentation
    //## @brief Shutdown the singleton. Workaround for bug #1446
    static void ShutdownSingleton();

    //##Documentation
    //## @brief A Container of objects that is used as a result set of GetSubset() query operations (Set of SmartPointers to DataTreeNodes).
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
    virtual void Add(mitk::DataTreeNode* node, const mitk::DataStorage::SetOfObjects* parents = NULL) = 0;

    //##Documentation
    //## @brief Convenience method to add a node that has one parent
    //##
    void Add(mitk::DataTreeNode* node, mitk::DataTreeNode* parent);

    //##Documentation
    //## @brief Removes node from the DataStorage
    //##
    virtual void Remove(const mitk::DataTreeNode* node) = 0;

    //##Documentation
    //## @brief Removes a set of nodes from the DataStorage
    //##
    void Remove(const mitk::DataStorage::SetOfObjects* nodes);

    //##Documentation
    //## @brief returns a set of data objects that meet the given condition(s)
    //##
    //## GetSubset returns a set of objects  with a specific data type that meet the condition(s)
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
    SetOfObjects::ConstPointer GetSubset(const NodePredicateBase& condition) const;

    //##Documentation
    //## @brief returns a set of source objects for a given node that meet the given condition(s).
    //##
    virtual SetOfObjects::ConstPointer GetSources(const mitk::DataTreeNode* node, const NodePredicateBase* condition = NULL, bool onlyDirectSources = true) const = 0;

    //##Documentation
    //## @brief returns a set of derived objects for a given node.
    //##
    //## GetDerivations() returns a set of objects that are derived from the DataTreeNode node.
    //## This means, that node was used to create the returned objects. If the parameter
    //## onlyDirectDerivations is set to true (default value), only objects that directly have
    //## node as one of their source objects will be returned. Otherwise, objects that are
    //## derived from derivations of node are returned too.
    //## The derived objects can be filtered with a predicate object as described in the GetSubset()
    //## method by providing a predicate as the condition parameter.
    virtual SetOfObjects::ConstPointer GetDerivations(const mitk::DataTreeNode* node, const NodePredicateBase* condition = NULL, bool onlyDirectDerivations = true) const = 0;

    //##Documentation
    //## @brief returns a set of all data objects that are stored in the data storage
    //##
    virtual SetOfObjects::ConstPointer GetAll() const = 0;

    //##Documentation
    //## @brief Convenience method to get the first node that matches the predicate condition
    //##
    mitk::DataTreeNode* GetNode(const NodePredicateBase* condition = NULL) const;


    //##Documentation
    //## @brief Convenience method to get the first node with a given name
    //##
    mitk::DataTreeNode* GetNamedNode(const char* name) const;

    //##Documentation
    //## @brief Convenience method to get the first node with a given name that is derived from sourceNode
    //##
    mitk::DataTreeNode* GetNamedDerivedNode(const char* name, const mitk::DataTreeNode* sourceNode, bool onlyDirectDerivations = true) const;

    //##Documentation
    //## @brief Convenience method to get the first data object of a given data type with a given name
    //##
    template <class DataType>
    DataType* GetNamedObject(const char* name) const
    {
      if (name == NULL)
        return NULL;
      mitk::DataTreeNode* n = this->GetNamedNode(name);
      if (n == NULL)
        return NULL;
      else
        return dynamic_cast<DataType*>(n->GetData());
    }

    //##Documentation
    //## @brief Convenience method to get the first data object of a given data type with a given name that is derived from a specific node
    //##
    template <class DataType>
    DataType* GetNamedDerivedObject(const char* name, const mitk::DataTreeNode* sourceNode, bool onlyDirectDerivations = true) const
    {
      if (name == NULL)
        return NULL;
      mitk::DataTreeNode* n = this->GetNamedDerivedNode(name, sourceNode, onlyDirectDerivations);
      if (n == NULL)
        return NULL;
      else
        return dynamic_cast<DataType*>(n->GetData());
    }

    //##Documentation
    //## @brief Returns a list of used grouptags
    //##
    const DataTreeNode::GroupTagList GetGroupTags() const;

    /* Public Events */
    typedef Message1<const mitk::DataTreeNode*> DataStorageEvent;
    //##Documentation
    //## @brief AddEvent is emitted whenever a new node has been added to the DataStorage. 
    //##
    //## Observers should register to this event by calling myDataStorage->AddNodeEvent.AddListener(myObject, MyObject::MyMethod).
    //## After registering, myObject->MyMethod() will be called every time a new node has been added to the DataStorage.
    //## Observers should unregister by calling myDataStorage->AddNodeEvent.RemoveListener(myObject, MyObject::MyMethod).
    //## Note: AddEvents are _not_ emitted if a node is added to DataStorage by adding it to the the underlying DataTree!
    DataStorageEvent AddNodeEvent;

    //##Documentation
    //## @brief RemoveEvent is emitted directly before a node is removed from the DataStorage. 
    //##
    //## Observers should register to this event by calling myDataStorage->RemoveNodeEvent.AddListener(myObject, MyObject::MyMethod).
    //## After registering, myObject->MyMethod() will be called every time a new node has been added to the DataStorage.
    //## Observers should unregister by calling myDataStorage->RemoveNodeEvent.RemoveListener(myObject, MyObject::MyMethod).
    //## Note: RemoveEvents are also emitted if a node was removed from the DataStorage by deleting it from the underlying DataTree
    DataStorageEvent RemoveNodeEvent;

  protected:

    //TODO investigate removing friend declarations when DataStorage is
    //     independent of the DataTree
    friend class BaseRenderer;
    friend class Geometry2DDataVtkMapper3D;
    friend class RenderingManager;

    //##Documentation
    //## @brief  EmitAddNodeEvent emits the AddNodeEvent
    //##
    //## This method should be called by subclasses to emit the AddNodeEvent
    void EmitAddNodeEvent(const mitk::DataTreeNode* node);

    //##Documentation
    //## @brief  EmitRemoveNodeEvent emits the RemoveNodeEvent
    //##
    //## This method should be called by subclasses to emit the RemoveNodeEvent
    void EmitRemoveNodeEvent(const mitk::DataTreeNode* node);

    //##Documentation
    //## @brief Standard Constructor for ::New() instantiation
    DataStorage();
    //##Documentation
    //## @brief Standard Destructor
    virtual ~DataStorage();

    //##Documentation
    //## @brief Filters a SetOfObjects by the condition. If no condition is provided, the original set is returned
    SetOfObjects::ConstPointer FilterSetOfObjects(const SetOfObjects* set, const NodePredicateBase* condition) const;

    //##Documentation
    //## @brief Prints the contents of the DataStorage to os. Do not call directly, call ->Print() instead
    virtual void PrintSelf(std::ostream& os, itk::Indent indent) const;

    static mitk::DataStorage::Pointer s_Instance;
  };
} // namespace mitk

#endif /* MITKDATASTORAGE_H_HEADER_INCLUDED_ */
