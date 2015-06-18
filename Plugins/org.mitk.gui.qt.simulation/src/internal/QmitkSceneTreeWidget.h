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

#ifndef QmitkSceneTreeWidget_h
#define QmitkSceneTreeWidget_h

#include <QMap>
#include <QTreeWidget>
#include <sofa/simulation/common/MutationListener.h>

namespace sofa
{
  namespace core
  {
    namespace objectmodel
    {
      class Base;
      class BaseNode;
      class BaseObject;
    }
  }

  namespace simulation
  {
    class Node;
  }
}

class QmitkSceneTreeWidget : public QTreeWidget, public sofa::simulation::MutationListener
{
  Q_OBJECT

public:
  typedef sofa::core::objectmodel::Base Base;
  typedef sofa::core::objectmodel::BaseNode BaseNode;
  typedef sofa::core::objectmodel::BaseObject BaseObject;
  typedef sofa::simulation::Node Node;

  explicit QmitkSceneTreeWidget(QWidget* parent = nullptr);
  ~QmitkSceneTreeWidget();

  Base* GetBaseFromItem(QTreeWidgetItem* item) const;

  // QTreeWidget, QTreeView, and QAbstractItemView Interfaces /////////////////
  void clear();
  /////////////////////////////////////////////////////////////////////////////

  // MutationListener Interface ///////////////////////////////////////////////
  void addChild(Node* parent, Node* child) override;
  void removeChild(Node* parent, Node* child) override;
  void moveChild(Node* previous, Node* parent, Node* child) override;
  void addObject(Node* parent, BaseObject* object) override;
  void removeObject(Node* parent, BaseObject* object) override;
  void moveObject(Node* previous, Node* parent, BaseObject* object) override;
  void addSlave(BaseObject* master, BaseObject* slave) override;
  void removeSlave(BaseObject* master, BaseObject* slave) override;
  void moveSlave(BaseObject* previousMaster, BaseObject* master, BaseObject* slave) override;
  /////////////////////////////////////////////////////////////////////////////

private:
  void ClearMaps();
  void InsertIntoMaps(Base* base, QTreeWidgetItem* item);
  void RemoveFromMaps(Base* base);

  QMap<Base*, QTreeWidgetItem*> m_BaseItemMap;
  QMap<QTreeWidgetItem*, Base*> m_ItemBaseMap;
};

#endif
