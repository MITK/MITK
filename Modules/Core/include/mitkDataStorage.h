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

#ifndef MITKDATASTORAGE_H
#define MITKDATASTORAGE_H

#include "itkObject.h"
#include "itkSimpleFastMutexLock.h"
#include "itkVectorContainer.h"
#include "mitkDataNode.h"
#include "mitkGeometry3D.h"
#include "mitkMessage.h"
#include <MitkCoreExports.h>
#include <map>

namespace mitk
{
  class NodePredicateBase;
  class DataNode;
  class BaseRenderer;

  //##Documentation
  //## @brief Data management class that handles 'was created by' relations
  //##
  //## The DataStorage provides data storage and management functionality.
  //## It handles a 'was created by' relation by associating each data object with a
  //## set of source objects, that this object was created from.
  //## Thus, nodes are stored in a noncyclical directed graph data structure.
  //## If a new node is added to the DataStorage, AddNodeEvent is emitted.
  //## If a node is removed, RemoveNodeEvent is emitted.
  //##
  //##
  //## \ingroup DataStorage
  class MITKCORE_EXPORT DataStorage : public itk::Object
  {
  public:
    mitkClassMacroItkParent(DataStorage, itk::Object);

    //##Documentation
    //## @brief A Container of objects that is used as a result set of GetSubset() query operations (Set of
    //SmartPointers
    // to DataNodes).
    typedef itk::VectorContainer<unsigned int, DataNode::Pointer> SetOfObjects;

    //##Documentation
    //## @brief Adds a DataNode containing a data object to its internal storage
    //##
    //## This Method adds a new data object to the DataStorage. The new object is
    //## passed in the first parameter. The second parameter is a set
    //## of source objects, that were used to create this object. The new object will have
    //## a 'was created from' relation to its source objects.
    //## the addition of a new object will fire the notification mechanism.
    //## If the node parameter is nullptr or if the DataNode has already been added,
    //## an exception will be thrown.
    virtual void Add(DataNode *node, const DataStorage::SetOfObjects *parents = nullptr) = 0;

    //##Documentation
    //## @brief Convenience method to add a node that has one parent
    //##
    void Add(DataNode *node, DataNode *parent);

    //##Documentation
    //## @brief Removes node from the DataStorage
    //##
    virtual void Remove(const DataNode *node) = 0;

    //##Documentation
    //## @brief Checks if a node exists in the DataStorage
    //##
    virtual bool Exists(const DataNode *node) const = 0;

    //##Documentation
    //## @brief Removes a set of nodes from the DataStorage
    //##
    void Remove(const DataStorage::SetOfObjects *nodes);

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
    //## The method returns a set of SmartPointers to the DataNodes that fulfill the
    //## conditions. A set of all objects can be retrieved with the GetAll() method;
    SetOfObjects::ConstPointer GetSubset(const NodePredicateBase *condition) const;

    //##Documentation
    //## @brief returns a set of source objects for a given node that meet the given condition(s).
    //##
    virtual SetOfObjects::ConstPointer GetSources(const DataNode *node,
                                                  const NodePredicateBase *condition = nullptr,
                                                  bool onlyDirectSources = true) const = 0;

    //##Documentation
    //## @brief returns a set of derived objects for a given node.
    //##
    //## GetDerivations() returns a set of objects that are derived from the DataNode node.
    //## This means, that node was used to create the returned objects. If the parameter
    //## onlyDirectDerivations is set to true (default value), only objects that directly have
    //## node as one of their source objects will be returned. Otherwise, objects that are
    //## derived from derivations of node are returned too.
    //## The derived objects can be filtered with a predicate object as described in the GetSubset()
    //## method by providing a predicate as the condition parameter.
    virtual SetOfObjects::ConstPointer GetDerivations(const DataNode *node,
                                                      const NodePredicateBase *condition = nullptr,
                                                      bool onlyDirectDerivations = true) const = 0;

    //##Documentation
    //## @brief returns a set of all data objects that are stored in the data storage
    //##
    virtual SetOfObjects::ConstPointer GetAll() const = 0;

    //##Documentation
    //## @brief Convenience method to get the first node that matches the predicate condition
    //##
    DataNode *GetNode(const NodePredicateBase *condition = nullptr) const;

    //##Documentation
    //## @brief Convenience method to get the first node with a given name
    //##
    DataNode *GetNamedNode(const char *name) const;

    //##Documentation
    //## @brief Convenience method to get the first node with a given name
    //##
    DataNode *GetNamedNode(const std::string name) const { return this->GetNamedNode(name.c_str()); }
    //##Documentation
    //## @brief Convenience method to get the first node with a given name that is derived from sourceNode
    //##
    DataNode *GetNamedDerivedNode(const char *name,
                                  const DataNode *sourceNode,
                                  bool onlyDirectDerivations = true) const;

    //##Documentation
    //## @brief Convenience method to get the first data object of a given data type with a given name
    //##
    template <class DataType>
    DataType *GetNamedObject(const char *name) const
    {
      if (name == nullptr)
        return nullptr;
      DataNode *n = this->GetNamedNode(name);
      if (n == nullptr)
        return nullptr;
      else
        return dynamic_cast<DataType *>(n->GetData());
    }
    //##Documentation
    //## @brief Convenience method to get the first data object of a given data type with a given name
    //##
    template <class DataType>
    DataType *GetNamedObject(const std::string name) const
    {
      return this->GetNamedObject<DataType>(name.c_str());
    }

    //##Documentation
    //## @brief Convenience method to get the first data object of a given data type with a given name that is derived
    // from a specific node
    //##
    template <class DataType>
    DataType *GetNamedDerivedObject(const char *name,
                                    const DataNode *sourceNode,
                                    bool onlyDirectDerivations = true) const
    {
      if (name == nullptr)
        return nullptr;
      DataNode *n = this->GetNamedDerivedNode(name, sourceNode, onlyDirectDerivations);
      if (n == nullptr)
        return nullptr;
      else
        return dynamic_cast<DataType *>(n->GetData());
    }

    //##Documentation
    //## @brief Returns a list of used grouptags
    //##
    const DataNode::GroupTagList GetGroupTags() const;

    /*ITK Mutex */
    mutable itk::SimpleFastMutexLock m_MutexOne;

    /* Public Events */
    typedef Message1<const DataNode*> DataStorageEvent;
    //##Documentation
    //## @brief AddEvent is emitted whenever a new node has been added to the DataStorage.
    //##
    //## Observers should register to this event by calling myDataStorage->AddNodeEvent.AddListener(myObject,
    // MyObject::MyMethod).
    //## After registering, myObject->MyMethod() will be called every time a new node has been added to the DataStorage.
    //## Observers should unregister by calling myDataStorage->AddNodeEvent.RemoveListener(myObject,
    //MyObject::MyMethod).
    //## Note: AddEvents are _not_ emitted if a node is added to DataStorage by adding it to the the underlying
    //DataTree!

    // member variable is not needed to be locked in multi threaded scenarios since the DataStorageEvent is a typedef
    // for
    // a Message1 object which is thread safe
    DataStorageEvent AddNodeEvent;

    //##Documentation
    //## @brief RemoveEvent is emitted directly before a node is removed from the DataStorage.
    //##
    //## Observers should register to this event by calling myDataStorage->RemoveNodeEvent.AddListener(myObject,
    // MyObject::MyMethod).
    //## After registering, myObject->MyMethod() will be called every time a new node has been added to the DataStorage.
    //## Observers should unregister by calling myDataStorage->RemoveNodeEvent.RemoveListener(myObject,
    // MyObject::MyMethod).
    //## Note: RemoveEvents are also emitted if a node was removed from the DataStorage by deleting it from the
    //underlying
    // DataTree

    // member variable is not needed to be locked in multi threaded scenarios since the DataStorageEvent is a typedef
    // for
    // a Message1 object which is thread safe
    DataStorageEvent RemoveNodeEvent;

    //##Documentation
    //## @brief ChangedEvent is emitted directly after a node was changed.
    //##
    //## Observers should register to this event by calling myDataStorage->ChangedNodeEvent.AddListener(myObject,
    // MyObject::MyMethod).
    //## After registering, myObject->MyMethod() will be called every time a new node has been changed.
    //## Observers should unregister by calling myDataStorage->ChangedNodeEvent.RemoveListener(myObject,
    // MyObject::MyMethod).
    //## Internally the DataStorage listens to itk::ModifiedEvents on the nodes and forwards them
    //## to the listeners of this event.

    // member variable is not needed to be locked in multi threaded scenarios since the DataStorageEvent is a typedef
    // for
    // a Message1 object which is thread safe
    DataStorageEvent ChangedNodeEvent;

    //##Documentation
    //## @brief DeleteNodeEvent is emitted directly before a node is deleted.
    //##
    //## Observers should register to this event by calling myDataStorage->DeleteNodeEvent.AddListener(myObject,
    // MyObject::MyMethod).
    //## After registering, myObject->MyMethod() will be called when a node is deleted.
    //## Observers should unregister by calling myDataStorage->DeleteNodeEvent.RemoveListener(myObject,
    // MyObject::MyMethod).
    //## Internally the DataStorage listens to itk::DeleteEvents on the nodes and forwards them
    //## to the listeners of this event.

    // member variable is not needed to be locked in multi threaded scenarios since the DataStorageEvent is a typedef
    // for
    // a Message1 object which is thread safe
    DataStorageEvent DeleteNodeEvent;

    DataStorageEvent InteractorChangedNodeEvent;

    //##Documentation
    //## @brief Compute the axis-parallel bounding geometry of the input objects
    //##
    //## Throws std::invalid_argument exception if input is nullptr
    //## @param input set of objects of the DataStorage to be included in the bounding geometry
    //## @param boolPropertyKey if a BoolProperty with this boolPropertyKey exists for a node (for @a renderer)
    //## and is set to @a false, the node is ignored for the bounding-box calculation.
    //## @param renderer see @a boolPropertyKey
    //## @param boolPropertyKey2 a second condition that is applied additionally to @a boolPropertyKey
    TimeGeometry::ConstPointer ComputeBoundingGeometry3D(const SetOfObjects *input,
                                                         const char *boolPropertyKey = nullptr,
                                                         const BaseRenderer *renderer = nullptr,
                                                         const char *boolPropertyKey2 = nullptr) const;

    //##Documentation
    //## @brief Compute the axis-parallel bounding geometry of the data tree
    //## (bounding box, minimal spacing of the considered nodes, live-span)
    //##
    //## it -> an iterator to a data tree structure
    //## @param boolPropertyKey if a BoolProperty with this boolPropertyKey exists for a node (for @a renderer)
    //## and is set to @a false, the node is ignored for the bounding-box calculation.
    //## @param renderer see @a boolPropertyKey
    //## @param boolPropertyKey2 a second condition that is applied additionally to @a boolPropertyKey
    TimeGeometry::ConstPointer ComputeBoundingGeometry3D(const char *boolPropertyKey = nullptr,
                                                         const BaseRenderer *renderer = nullptr,
                                                         const char *boolPropertyKey2 = nullptr) const;

    //##Documentation
    //## @brief Compute the axis-parallel bounding geometry of all visible parts of the
    //## data tree bounding box, minimal spacing of the considered nodes, live-span)
    //##
    //## Simply calls ComputeBoundingGeometry3D(it, "visible", renderer, boolPropertyKey).
    //## it -> an iterator of a data tree structure
    //## @param renderer the reference to the renderer
    //## @param boolPropertyKey if a BoolProperty with this boolPropertyKey exists for a node (for @a renderer)
    //## and is set to @a false, the node is ignored for the bounding-box calculation.
    TimeGeometry::ConstPointer ComputeVisibleBoundingGeometry3D(const BaseRenderer *renderer = nullptr,
                                                                const char *boolPropertyKey = nullptr);

    //##Documentation
    //## @brief Compute the bounding box of data tree structure
    //## it -> an iterator to a data tree structure
    //## @param boolPropertyKey if a BoolProperty with this boolPropertyKey exists for a node (for @a renderer)
    //## and is set to @a false, the node is ignored for the bounding-box calculation.
    //## @param renderer see @a boolPropertyKey
    //## @param boolPropertyKey2 a second condition that is applied additionally to @a boolPropertyKey
    BoundingBox::Pointer ComputeBoundingBox(const char *boolPropertyKey = nullptr,
                                            const BaseRenderer *renderer = nullptr,
                                            const char *boolPropertyKey2 = nullptr);

    //##Documentation
    //## \brief Compute the bounding box of all visible parts of the data tree structure, for general
    //## rendering or renderer specific visibility property checking
    //##
    //## Simply calls ComputeBoundingBox(it, "visible", renderer, boolPropertyKey).
    //## it -> an iterator of a data tree structure
    //## @param renderer the reference to the renderer
    //## @param boolPropertyKey if a BoolProperty with this boolPropertyKey exists for a node (for @a renderer)
    //## and is set to @a false, the node is ignored for the bounding-box calculation.
    BoundingBox::Pointer ComputeVisibleBoundingBox(const BaseRenderer *renderer = nullptr,
                                                   const char *boolPropertyKey = nullptr)
    {
      return ComputeBoundingBox("visible", renderer, boolPropertyKey);
    }

    //##Documentation
    //## @brief Compute the time-bounds of the contents of a data tree structure
    //##
    //## The methods returns only [-infinity, +infinity], if all data-objects have an infinite live-span. Otherwise,
    //## all data-objects with infinite live-span are ignored.
    //## it -> an iterator to a data tree structure
    //## @param boolPropertyKey if a BoolProperty with this boolPropertyKey exists for a node (for @a renderer)
    //## and is set to @a false, the node is ignored for the time-bounds calculation.
    //## @param renderer see @a boolPropertyKey
    //## @param boolPropertyKey2 a second condition that is applied additionally to @a boolPropertyKey
    TimeBounds ComputeTimeBounds(const char *boolPropertyKey,
                                 const BaseRenderer *renderer,
                                 const char *boolPropertyKey2);

    //##Documentation
    //## @brief Compute the time-bounds of all visible parts of the data tree structure, for general
    //## rendering or renderer specific visibility property checking
    //##
    //## The methods returns only [-infinity, +infinity], if all data-objects have an infinite live-span. Otherwise,
    //## all data-objects with infinite live-span are ignored.
    //## Simply calls ComputeTimeBounds(it, "visible", renderer, boolPropertyKey).
    //## @param it an iterator to a data tree structure
    //## @param boolPropertyKey if a BoolProperty with this boolPropertyKey exists for a node (for @a renderer)
    //## and is set to @a false, the node is ignored for the time-bounds calculation.
    //## @param renderer see @a boolPropertyKey
    TimeBounds ComputeTimeBounds(const BaseRenderer *renderer, const char *boolPropertyKey)
    {
      return ComputeTimeBounds("visible", renderer, boolPropertyKey);
    }

    //##Documentation
    //## @brief Defines whether or not NodeChangedEvent is invoked .
    //##
    //## This method can be used to set m_BlockNodeModifiedEvents.
    //##
    //## If this flag is true, NodeChangedEvent is not invoked when a
    //## DataNode is modified. This might be undesired when setting
    //## many properties on a datanode and you do not want anyone to
    //## react.
    void BlockNodeModifiedEvents(bool block);

  protected:
    //##Documentation
    //## @brief  EmitAddNodeEvent emits the AddNodeEvent
    //##
    //## This method should be called by subclasses to emit the AddNodeEvent
    void EmitAddNodeEvent(const DataNode *node);

    //##Documentation
    //## @brief  EmitRemoveNodeEvent emits the RemoveNodeEvent
    //##
    //## This method should be called by subclasses to emit the RemoveNodeEvent
    void EmitRemoveNodeEvent(const DataNode *node);

    void OnNodeInteractorChanged(itk::Object *caller, const itk::EventObject &event);

    //##Documentation
    //## @brief  OnNodeModified listens to modified events of DataNodes.
    //##
    //## The node is hidden behind the caller parameter, which has to be casted first.
    //## If the cast succeeds the ChangedNodeEvent is emitted with this node.
    void OnNodeModifiedOrDeleted(const itk::Object *caller, const itk::EventObject &event);

    //##Documentation
    //## @brief  Adds a Modified-Listener to the given Node.
    void AddListeners(const DataNode *_Node);

    //##Documentation
    //## @brief  Removes a Modified-Listener from the given Node.
    void RemoveListeners(const DataNode *_Node);

    //##Documentation
    //## @brief  Saves Modified-Observer Tags for each node in order to remove the event listeners again.
    std::map<const DataNode *, unsigned long> m_NodeModifiedObserverTags;

    std::map<const DataNode *, unsigned long> m_NodeInteractorChangedObserverTags;

    //##Documentation
    //## @brief  Saves Delete-Observer Tags for each node in order to remove the event listeners again.
    std::map<const DataNode *, unsigned long> m_NodeDeleteObserverTags;

    //##Documentation
    //## @brief If this class changes nodes itself, set this to TRUE in order
    //## to suppress NodeChangedEvent to be emitted.
    bool m_BlockNodeModifiedEvents;

    //##Documentation
    //## @brief Standard Constructor for ::New() instantiation
    DataStorage();
    //##Documentation
    //## @brief Standard Destructor
    ~DataStorage() override;

    //##Documentation
    //## @brief Filters a SetOfObjects by the condition. If no condition is provided, the original set is returned
    SetOfObjects::ConstPointer FilterSetOfObjects(const SetOfObjects *set, const NodePredicateBase *condition) const;

    //##Documentation
    //## @brief Prints the contents of the DataStorage to os. Do not call directly, call ->Print() instead
    void PrintSelf(std::ostream &os, itk::Indent indent) const override;
  };

  //##Documentation
  //## @brief returns the topmost visible node of a given list of nodes.
  //## The function returns a node that is visible and has the highest layer of a set of given nodes.
  //## The property list, which is used to find the visibility- and layer-property is is specified by the
  //## given base renderer.
  //##
  MITKCORE_EXPORT DataNode::Pointer FindTopmostVisibleNode(const DataStorage::SetOfObjects::ConstPointer nodes,
                                                           const Point3D worldPosition,
                                                           const TimePointType timePoint,
                                                           const BaseRenderer* baseRender);
} // namespace mitk

#endif // MITKDATASTORAGE_H
