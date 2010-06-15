/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-07-14 19:11:20 +0200 (Tue, 14 Jul 2009) $
Version:   $Revision: 18127 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef mitkDataStorageSelection_h
#define mitkDataStorageSelection_h

#include <mitkDataStorage.h>
#include "MitkExtExports.h"
#include <mitkNodePredicateBase.h>
#include <itkEventObject.h>
#include <mitkCommon.h>
#include <set>

namespace mitk
{
  class BaseProperty;
  class PropertyList;

  class MitkExt_EXPORT DataStorageSelection: public itk::Object
  {
  public:
    typedef std::vector<mitk::DataNode*> Nodes;
    typedef Message1<const mitk::DataNode*> DataNodeEvent;

    DataNodeEvent NodeChanged;
    DataNodeEvent NodeAdded;
    DataNodeEvent NodeRemoved;
    Message2<const mitk::DataNode*, const mitk::BaseProperty*> PropertyChanged;

    mitkClassMacro(DataStorageSelection, itk::Object);

    mitkNewMacro2Param(DataStorageSelection, DataStorage*, bool);
    mitkNewMacro3Param(DataStorageSelection, DataStorage*, NodePredicateBase*, bool);

    protected:
      DataStorageSelection(mitk::DataStorage* _DataStorage, bool _AutoAddNodes);
      DataStorageSelection(mitk::DataStorage* _DataStorage
        , mitk::NodePredicateBase* _Predicate, bool _AutoAddNodes);

    public:
      virtual ~DataStorageSelection();

      ///
      /// Get the DataStorage.
      ///
      mitk::DataStorage::Pointer GetDataStorage() const;
      ///
      /// Get the predicate.
      ///
      mitk::NodePredicateBase::Pointer GetPredicate() const;
      ///
      /// Returns the size of this selection
      ///
      unsigned int GetSize() const;
      ///
      /// Get node at a specific model index.
      ///
      mitk::DataNode::Pointer GetNode(unsigned int index) const;
      ///
      /// Returns the first node, same as calling GetNode(0)
      ///
      mitk::DataNode::Pointer GetNode() const;
      ///
      /// Returns a copy of the node-vector
      ///
      std::vector<mitk::DataNode*> GetNodes() const;
      ///
      /// \see m_AutoAddNodes
      ///
      bool DoesAutoAddNodes() const;
    public:
      ///
      /// Removes all nodes, sets node as new first element
      ///
      DataStorageSelection& operator=(mitk::DataNode* node);
      ///
      /// Removes all nodes, sets node as new first element
      ///
      DataStorageSelection& operator=(mitk::DataNode::Pointer node);
      ///
      /// Sets the DataStorage.
      ///
      virtual void SetDataStorage(mitk::DataStorage* _DataStorage);
      ///
      /// Sets the predicate. <b>QmitkDataStorageTableModel is owner of the predicate!</b>
      ///
      virtual void SetPredicate(mitk::NodePredicateBase* _Predicate);
      ///
      /// Add a node (if not already there)
      ///
      virtual void AddNode(const mitk::DataNode* node);
      ///
      /// Removes a node
      ///
      virtual void RemoveNode(const mitk::DataNode* node);
      ///
      /// Removes a node
      ///
      virtual void RemoveAllNodes();
      ///
      /// Called whenever an itk Object this class holds gets deleted or modified
      ///
      virtual void ObjectChanged(const itk::Object *caller, const itk::EventObject &event);
    protected:
      ///
      /// Find a node in the list by the given prop
      ///
      mitk::DataNode::Pointer FindNode(const mitk::BaseProperty* prop) const;
      ///
      /// Find a node in the list by the given proplist
      ///
      mitk::DataNode::Pointer FindNode(const mitk::PropertyList* propList) const;
      ///
      /// Removes all nodes and fill the vector again
      ///
      void Reset();
      ///
      /// If a node is already in this list, all listeners will be removed
      ///
      void RemoveListener(mitk::DataNode* node);
      ///
      /// Adds listeners for modified/delete event, for the propertylists
      /// modified/delete and for the modified/delete event of each property
      /// and stores listener tags
      ///
      void AddListener(mitk::DataNode* node);
    protected:
      ///
      /// Pointer to the DataStorage from which the nodes are selected
      ///
      mitk::DataStorage* m_DataStorage;
      ///
      /// DeleteTag for the DataStorage
      ///
      unsigned int m_DataStorageDeletedTag;
      ///
      /// Holds the predicate that defines this SubSet of Nodes. If m_Predicate
      /// is NULL all Nodes will be selected.
      ///
      mitk::NodePredicateBase::Pointer m_Predicate;
      ///
      /// Holds all selected Nodes.
      ///
      std::vector<mitk::DataNode*> m_Nodes;
      ///
      /// \brief Maps a node to a modified observer tag.
      ///
      std::map<mitk::DataNode*, unsigned long> m_NodeModifiedObserverTags;
      ///
      /// \brief Maps a propertylist to a modified observer tag.
      ///
      std::map<mitk::PropertyList*, unsigned long> m_PropertyListModifiedObserverTags;
      ///
      /// \brief Maps a propertylist to a delete observer tag.
      ///
      std::map<mitk::PropertyList*, unsigned long> m_PropertyListDeletedObserverTags;
      ///
      /// \brief Maps a property to  a modified observer tag.
      ///
      std::map<mitk::BaseProperty*, unsigned long> m_PropertyModifiedObserverTags;
      ///
      /// \brief Maps a property to a delete observer tag.
      ///
      std::map<mitk::BaseProperty*, unsigned long> m_PropertyDeletedObserverTags;
      ///
      /// If set to true no event processing will be performed
      ///
      bool m_SelfCall;
      ///
      /// Saves if new nodes are automatically added to this selection
      ///
      bool m_AutoAddNodes;
  };

}

#endif // mitkDataStorageSelection_h
