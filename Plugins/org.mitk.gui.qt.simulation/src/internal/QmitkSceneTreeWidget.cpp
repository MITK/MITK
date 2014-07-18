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

#include "QmitkSceneTreeWidget.h"
#include <sofa/core/collision/CollisionGroupManager.h>
#include <sofa/core/collision/ContactManager.h>
#include <sofa/simulation/common/Colors.h>
#include <sofa/simulation/common/Node.h>

template <class T>
static inline T* as(QmitkSceneTreeWidget::Base* base)
{
  return dynamic_cast<T*>(base);
}

template <class T>
static inline bool is(QmitkSceneTreeWidget::Base* base)
{
  return dynamic_cast<T*>(base) != NULL;
}

template <class T1, class T2>
static inline bool is(QmitkSceneTreeWidget::Base* base)
{
  return is<T1>(base) || is<T2>(base);
}

template <class T1, class T2, class T3, class T4, class T5>
static inline bool is(QmitkSceneTreeWidget::Base* base)
{
  return is<T1, T2>(base) || is<T3, T4>(base) || is<T5>(base);
}

static inline bool isBaseInteractionForceField(QmitkSceneTreeWidget::Base* base)
{
  sofa::core::behavior::BaseInteractionForceField* iff = dynamic_cast<sofa::core::behavior::BaseInteractionForceField*>(base);
  return iff != NULL && iff->getMechModel1() != iff->getMechModel2();
}

static inline bool isMechanicalMapping(QmitkSceneTreeWidget::Base* base)
{
  sofa::core::BaseMapping* mm = dynamic_cast<sofa::core::BaseMapping*>(base);
  return mm != NULL && mm->isMechanical();
}

static QRgb GetColor(QmitkSceneTreeWidget::Base* base)
{
  using namespace sofa::core;
  using namespace sofa::core::behavior;
  using namespace sofa::core::collision;
  using namespace sofa::core::loader;
  using namespace sofa::core::objectmodel;
  using namespace sofa::core::topology;
  using namespace sofa::core::visual;
  using namespace sofa::simulation::Colors;

  QString hexColor;

  if (is<BaseNode>(base))
  {
    hexColor = COLOR[NODE];
  }
  else if (is<BaseObject>(base))
  {
    if (is<ContextObject>(base))
      hexColor = COLOR[sofa::simulation::Colors::CONTEXT];
    else if (is<BehaviorModel>(base))
      hexColor = COLOR[BMODEL];
    else if (is<CollisionModel>(base))
      hexColor = COLOR[CMODEL];
    else if (is<BaseMechanicalState>(base))
      hexColor = COLOR[MMODEL];
    else if (is<BaseProjectiveConstraintSet>(base))
      hexColor = COLOR[PROJECTIVECONSTRAINTSET];
    else if (is<BaseConstraintSet>(base))
      hexColor = COLOR[CONSTRAINTSET];
    else if (is<BaseMass>(base))
      hexColor = COLOR[MASS];
    else if (isBaseInteractionForceField(base))
      hexColor = COLOR[IFFIELD];
    else if (is<BaseForceField>(base))
      hexColor = COLOR[FFIELD];
    else if (is<BaseAnimationLoop, OdeSolver>(base))
      hexColor = COLOR[SOLVER];
    else if (is<Pipeline, Intersection, Detection, ContactManager, CollisionGroupManager>(base))
      hexColor = COLOR[COLLISION];
    else if (isMechanicalMapping(base))
      hexColor = COLOR[MMAPPING];
    else if (is<BaseMapping>(base))
      hexColor = COLOR[MAPPING];
    else if (is<Topology, BaseTopologyObject>(base))
      hexColor = COLOR[TOPOLOGY];
    else if (is<BaseLoader>(base))
      hexColor = COLOR[LOADER];
    else if (is<ConfigurationSetting>(base))
      hexColor = COLOR[CONFIGURATIONSETTING];
    else if (is<VisualModel>(base))
      hexColor = COLOR[VMODEL];
    else
      hexColor = COLOR[OBJECT];
  }
  else
  {
    hexColor = "#000";
  }

  QColor color;
  color.setNamedColor(hexColor);

  return color.rgb();
}

static QPixmap ReplaceColor(const QPixmap& pixmap, QRgb from, QRgb to)
{
  QImage image = pixmap.toImage();

  const int width = image.width();
  const int height = image.height();
  int x, y;

  for (y = 0; y < height; ++y)
  {
    for (x = 0; x < width; ++x)
    {
      if (image.pixel(x, y) == from)
        image.setPixel(x, y, to);
    }
  }

  return QPixmap::fromImage(image);
}

static inline QIcon CreateObjectIcon(QmitkSceneTreeWidget::Base* base)
{
  return QIcon(ReplaceColor(QPixmap(":/Simulation/Object"), 0xff00ff00, GetColor(base)));
}

static inline QIcon CreateNodeIcon(QmitkSceneTreeWidget::BaseNode* node)
{
  return QIcon(ReplaceColor(QPixmap(":/Simulation/Node"), 0xff00ff00, GetColor(node)));
}

static inline QIcon CreateSlaveIcon(QmitkSceneTreeWidget::Base* base)
{
  return QIcon(ReplaceColor(QPixmap(":/Simulation/Slave"), 0xff00ff00, GetColor(base)));
}

static inline QString GetName(QmitkSceneTreeWidget::Base* base)
{
  return QString::fromStdString(base->getName());
}

static inline QString GetClassName(QmitkSceneTreeWidget::Base* base)
{
  return QString::fromStdString(base->getClassName());
}

QmitkSceneTreeWidget::QmitkSceneTreeWidget(QWidget* parent)
  : QTreeWidget(parent)
{
}

QmitkSceneTreeWidget::~QmitkSceneTreeWidget()
{
}

void QmitkSceneTreeWidget::clear()
{
  QTreeWidgetItem* rootItem = this->topLevelItem(0);

  if (rootItem != NULL)
    this->removeChild(NULL, as<Node>(m_ItemBaseMap[rootItem]));

  this->ClearMaps();

  QTreeWidget::clear();
}

void QmitkSceneTreeWidget::addChild(Node* parent, Node* child)
{
  assert(child != NULL && "Child node is NULL!");
  assert(!m_BaseItemMap.contains(child) && "TODO: Support nodes with multiple parents!");

  QTreeWidgetItem* item;

  if (parent == NULL)
  {
    item = new QTreeWidgetItem(QStringList() << GetName(child));
    this->addTopLevelItem(item);
  }
  else
  {
    assert(m_BaseItemMap.contains(parent) && "Unknown parent node!");
    item = new QTreeWidgetItem(m_BaseItemMap[parent], QStringList() << GetName(child));
  }

  item->setIcon(0, CreateNodeIcon(child));
  this->InsertIntoMaps(child, item);

  MutationListener::addChild(parent, child);
}

void QmitkSceneTreeWidget::removeChild(Node* parent, Node* child)
{
  assert(child != NULL && "Child node is NULL!");
  assert(m_BaseItemMap.contains(child) && "Child node has already been removed!");

  MutationListener::removeChild(parent, child);

  if (parent == NULL)
  {
    delete m_BaseItemMap[child];
  }
  else
  {
    assert(m_BaseItemMap.contains(parent) && "Unknown parent node!");
    m_BaseItemMap[parent]->removeChild(m_BaseItemMap[child]);
  }

  this->RemoveFromMaps(child); 
}

void QmitkSceneTreeWidget::moveChild(Node* previous, Node* parent, Node* child)
{
  if (previous == NULL)
  {
    this->addChild(parent, child);
  }
  else if (parent == NULL)
  {
    this->removeChild(previous, child);
  }
  else
  {
    assert(child != NULL && "Child node is NULL!");
    assert(m_BaseItemMap.contains(previous) && "Unknown previous parent node!");
    assert(m_BaseItemMap.contains(parent) && "Unknown parent node!");
    assert(m_BaseItemMap.contains(child) && "Unknown child node!");

    QTreeWidgetItem* previousItem = m_BaseItemMap[previous];
    m_BaseItemMap[parent]->addChild(previousItem->takeChild(previousItem->indexOfChild(m_BaseItemMap[child])));
  }
}

void QmitkSceneTreeWidget::addObject(Node* parent, BaseObject* object)
{
  assert(parent != NULL && "Parent node is NULL!");
  assert(object != NULL && "Object is NULL!");
  assert(m_BaseItemMap.contains(parent) && "Unknown parent node!");
  // assert(!m_BaseItemMap.contains(object) && "Object has already been added!");

  if (!m_BaseItemMap.contains(object))
  {
    QTreeWidgetItem* item = new QTreeWidgetItem(m_BaseItemMap[parent], QStringList() << GetName(object));
    item->setToolTip(0, GetClassName(object));
    item->setIcon(0, CreateObjectIcon(object));
    this->InsertIntoMaps(object, item);
  }

  MutationListener::addObject(parent, object);
}

void QmitkSceneTreeWidget::removeObject(Node* parent, BaseObject* object)
{
  assert(parent != NULL && "Parent node is NULL!");
  assert(object != NULL && "Object is NULL!");
  assert(m_BaseItemMap.contains(parent) && "Unknown parent node!");
  assert(m_BaseItemMap.contains(object) && "Object has already been removed!");

  MutationListener::removeObject(parent, object);

  m_BaseItemMap[parent]->removeChild(m_BaseItemMap[object]);
  this->RemoveFromMaps(object);  
}

void QmitkSceneTreeWidget::moveObject(Node* previous, Node* parent, BaseObject* object)
{
  if (previous == NULL)
  {
    this->addObject(parent, object);
  }
  else if (parent == NULL)
  {
    this->removeObject(previous, object);
  }
  else
  {
    assert(object != NULL && "Object is NULL!");
    assert(m_BaseItemMap.contains(previous) && "Unknown previous parent node!");
    assert(m_BaseItemMap.contains(parent) && "Unknown parent node!");
    assert(m_BaseItemMap.contains(object) && "Unknown object!");

    QTreeWidgetItem* previousItem = m_BaseItemMap[previous];
    m_BaseItemMap[parent]->addChild(previousItem->takeChild(previousItem->indexOfChild(m_BaseItemMap[object])));
  }
}

void QmitkSceneTreeWidget::addSlave(BaseObject* master, BaseObject* slave)
{
  assert(master != NULL && "Master object is NULL!");
  assert(slave != NULL && "Slave object is NULL!");
  assert(m_BaseItemMap.contains(master) && "Unknown master object!");
  assert(!m_BaseItemMap.contains(slave) && "Slave object has already been added!");

  QTreeWidgetItem* item = new QTreeWidgetItem(m_BaseItemMap[master], QStringList() << GetName(slave));
  item->setToolTip(0, GetClassName(slave));
  item->setIcon(0, CreateSlaveIcon(slave));
  this->InsertIntoMaps(slave, item);

  MutationListener::addSlave(master, slave);
}

void QmitkSceneTreeWidget::removeSlave(BaseObject* master, BaseObject* slave)
{
  assert(master != NULL && "Master object is NULL!");
  assert(slave != NULL && "Slave object is NULL!");
  assert(m_BaseItemMap.contains(master) && "Unknown master object!");
  assert(m_BaseItemMap.contains(slave) && "Slave object has already been removed!");

  MutationListener::removeSlave(master, slave);

  m_BaseItemMap[master]->removeChild(m_BaseItemMap[slave]);
  this->RemoveFromMaps(slave);  
}

void QmitkSceneTreeWidget::moveSlave(BaseObject* previousMaster, BaseObject* master, BaseObject* slave)
{
  if (previousMaster == NULL)
  {
    this->addSlave(master, slave);
  }
  else if (master == NULL)
  {
    this->removeSlave(previousMaster, slave);
  }
  else
  {
    assert(slave != NULL && "Slave object is NULL!");
    assert(m_BaseItemMap.contains(previousMaster) && "Unknown previous master object!");
    assert(m_BaseItemMap.contains(master) && "Unknown master object!");
    assert(m_BaseItemMap.contains(slave) && "Unknown slave object!");

    QTreeWidgetItem* previousMasterItem = m_BaseItemMap[previousMaster];
    m_BaseItemMap[master]->addChild(previousMasterItem->takeChild(previousMasterItem->indexOfChild(m_BaseItemMap[slave])));
  }
}

QmitkSceneTreeWidget::Base* QmitkSceneTreeWidget::GetBaseFromItem(QTreeWidgetItem* item) const
{
  return m_ItemBaseMap.contains(item)
    ? m_ItemBaseMap[item]
    : NULL;
}

void QmitkSceneTreeWidget::ClearMaps()
{
  m_BaseItemMap.clear();
  m_ItemBaseMap.clear();
}

void QmitkSceneTreeWidget::InsertIntoMaps(Base* base, QTreeWidgetItem* item)
{
  m_BaseItemMap.insert(base, item);
  m_ItemBaseMap.insert(item, base);
}

void QmitkSceneTreeWidget::RemoveFromMaps(Base* base)
{
  m_ItemBaseMap.remove(m_BaseItemMap[base]);
  m_BaseItemMap.remove(base);
}
