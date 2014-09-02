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

#include "QmitkBoundingObjectWidget.h"

#include <mitkCone.h>
#include <mitkCylinder.h>
#include <mitkCuboid.h>
#include <mitkEllipsoid.h>
#include <mitkAffineInteractor.h>
#include <mitkGlobalInteraction.h>
#include <mitkNodePredicateProperty.h>
#include <mitkLine.h>
#include <mitkPlaneGeometry.h>

#include <QPushButton>
#include <QCheckBox>
#include <QBoxLayout>
#include <QStringList>
#include <QInputDialog>

QmitkBoundingObjectWidget::QmitkBoundingObjectWidget (QWidget* parent, Qt::WindowFlags f ):QWidget( parent, f ),
m_DataStorage(NULL),
m_lastSelectedItem(NULL),
m_lastAffineObserver(0),
m_ItemNodeMap(),
m_BoundingObjectCounter(1)
{

  QBoxLayout* mainLayout = new QVBoxLayout(this);

  QHBoxLayout* buttonLayout = new QHBoxLayout();

  QStringList boList;
  boList << tr("add") << tr("cube") << tr("cone") << tr("ellipse") << tr("cylinder");
  m_addComboBox = new QComboBox();
  m_addComboBox->addItems(boList);
  m_addComboBox->setItemIcon(1, QIcon(":/QmitkWidgetsExt/btnCube.xpm"));
  m_addComboBox->setItemIcon(2, QIcon(":/QmitkWidgetsExt/btnPyramid.xpm"));
  m_addComboBox->setItemIcon(3, QIcon(":/QmitkWidgetsExt/btnEllipsoid.xpm"));
  m_addComboBox->setItemIcon(4, QIcon(":/QmitkWidgetsExt/btnCylinder.xpm"));

  buttonLayout->addWidget(m_addComboBox);

  m_DelButton = new QPushButton("del");
  buttonLayout->addWidget(m_DelButton);


  m_SaveButton = new QPushButton("save");
  buttonLayout->addWidget(m_SaveButton);
  m_SaveButton->setEnabled(false);

  m_LoadButton = new QPushButton("load");
  buttonLayout->addWidget(m_LoadButton);
  m_LoadButton->setEnabled(false);

  m_TreeWidget = new QTreeWidget(this);
  m_TreeWidget->setColumnCount(3);
  QStringList sList;
  sList << tr("name") << tr("inverted") << tr("visible");
  m_TreeWidget->setHeaderLabels(sList);
  m_TreeWidget->setColumnWidth(0, 250);
  m_TreeWidget->setColumnWidth(1, 50);
  m_TreeWidget->setColumnWidth(2, 50);
  m_TreeWidget->setAutoScroll(true);
  m_TreeWidget->setSelectionMode(QAbstractItemView::SingleSelection);

  mainLayout->addWidget(m_TreeWidget);
  mainLayout->addLayout(buttonLayout);

  connect( m_addComboBox , SIGNAL(currentIndexChanged(int)), this, SLOT(CreateBoundingObject(int)) );
  connect( m_TreeWidget, SIGNAL(itemSelectionChanged()), this, SLOT(SelectionChanged()) );
  connect( m_DelButton, SIGNAL(clicked()), this, SLOT(OnDelButtonClicked()) );

  connect(m_TreeWidget, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), this, SLOT(OnItemDoubleClicked(QTreeWidgetItem*, int)) );
  connect(m_TreeWidget, SIGNAL(itemChanged(QTreeWidgetItem*, int)), this, SLOT(OnItemDataChanged(QTreeWidgetItem*, int)) );

}

QmitkBoundingObjectWidget::~QmitkBoundingObjectWidget()
{

}

void QmitkBoundingObjectWidget::setEnabled(bool flag)
{
  ItemNodeMapType::iterator it = m_ItemNodeMap.begin();
  while( it != m_ItemNodeMap.end())
  {
    mitk::DataNode* node = it->second;
    QTreeWidgetItem* item = it->first;

    if (flag)
      node->SetVisibility(item->checkState(2));
    else
      node->SetVisibility(flag);
    ++it;
  }

  QWidget::setEnabled(flag);
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkBoundingObjectWidget::SelectionChanged()
{

  QList<QTreeWidgetItem*> selectedItems = m_TreeWidget->selectedItems();
  if (selectedItems.size() < 1)
    return;

  QTreeWidgetItem* selectedItem = selectedItems.first();

  if (selectedItem == m_lastSelectedItem)
    return;

  if (m_lastSelectedItem != NULL)
  {
    m_TreeWidget->closePersistentEditor(m_lastSelectedItem, 0);

    ItemNodeMapType::iterator it = m_ItemNodeMap.find(m_lastSelectedItem);

    if (it != m_ItemNodeMap.end())
    {
      mitk::DataNode* last_node = it->second;

      //remove observer
      last_node->RemoveObserver(m_lastAffineObserver);

      //get and remove interactor
      mitk::AffineInteractor::Pointer last_interactor = dynamic_cast<mitk::AffineInteractor*> (last_node->GetInteractor());
      if (last_interactor)
        mitk::GlobalInteraction::GetInstance()->RemoveInteractor(last_interactor);
    }
  }

  ItemNodeMapType::iterator it = m_ItemNodeMap.find(selectedItem);
  if (it == m_ItemNodeMap.end())
    return;

  mitk::DataNode* new_node = it->second;

  mitk::AffineInteractor::Pointer new_interactor = mitk::AffineInteractor::New("AffineInteractions ctrl-drag", new_node);
  new_node->SetInteractor(new_interactor);

  mitk::GlobalInteraction::GetInstance()->AddInteractor(new_interactor);

  //create observer for node
  itk::ReceptorMemberCommand<QmitkBoundingObjectWidget>::Pointer command = itk::ReceptorMemberCommand<QmitkBoundingObjectWidget>::New();
  command->SetCallbackFunction(this, &QmitkBoundingObjectWidget::OnBoundingObjectModified);
  m_lastAffineObserver = new_node->AddObserver(mitk::AffineInteractionEvent(), command);

  m_lastSelectedItem = selectedItem;
}

void QmitkBoundingObjectWidget::AddItem(mitk::DataNode* node)
{
  mitk::BoundingObject* boundingObject;

  boundingObject = dynamic_cast<mitk::BoundingObject*> (node->GetData());

  std::string name;
  node->GetStringProperty("name", name);

  if (boundingObject)
  {
    QTreeWidgetItem* item = new QTreeWidgetItem();
    item->setData(0, Qt::EditRole,  QString::fromLocal8Bit(name.c_str()));
    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsUserCheckable);

    //checkbox for positive flag
    item->setData(1, Qt::CheckStateRole, tr(""));
    item->setCheckState(1, Qt::Unchecked);

    //checkbox for visibleflag
    item->setData(2, Qt::CheckStateRole, tr(""));
    item->setCheckState(2, Qt::Checked);

    m_TreeWidget->addTopLevelItem(item);

    m_ItemNodeMap.insert(std::make_pair(item, node));

    m_TreeWidget->selectAll();
    QList<QTreeWidgetItem*> items = m_TreeWidget->selectedItems();
    for( int i = 0; i<items.size(); i++)
    {
      m_TreeWidget->setItemSelected(items.at(i), false);
    }

    m_TreeWidget->setItemSelected(item, true);


  }
  else
    MITK_ERROR << name << " is not a bounding object or does not exist in data storage" << endl;
}

void QmitkBoundingObjectWidget::OnItemDoubleClicked(QTreeWidgetItem* item, int col)
{
  if (col == 0)
  {
    m_TreeWidget->openPersistentEditor(item, col);
  }
}

void QmitkBoundingObjectWidget::OnItemDataChanged(QTreeWidgetItem *item, int col)
{
  if (m_ItemNodeMap.size() < 1)
    return;

  ItemNodeMapType::iterator it = m_ItemNodeMap.find(item);
  if (it == m_ItemNodeMap.end())
    return;

  mitk::DataNode* node = it->second;

  //name
  if (col == 0)
  {
    m_TreeWidget->closePersistentEditor(item, col);
    node->SetName(item->text(0).toLocal8Bit().data());
  }
  //positive
  else if (col == 1)
  {
    mitk::BoundingObject* boundingObject = dynamic_cast<mitk::BoundingObject*> (node->GetData());
    if (boundingObject)
      boundingObject->SetPositive(!(item->checkState(1)));
    emit BoundingObjectsChanged();
  }
  //visible
  else if (col == 2)
  {
    node->SetVisibility(item->checkState(2));
  }

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();


}

void QmitkBoundingObjectWidget::RemoveItem()
{
  //selection mode is set to single selection, so there should not be more than one selected item
  QList <QTreeWidgetItem*> selectedItems = m_TreeWidget->selectedItems();
  QTreeWidgetItem* item = selectedItems.first();
  QString str = item->text(0);

  ItemNodeMapType::iterator it = m_ItemNodeMap.find(item);

  if (it == m_ItemNodeMap.end())
    return;

  mitk::DataNode* node = it->second;
  mitk::BoundingObject* boundingObject;

  if (node)
  {
    boundingObject = dynamic_cast<mitk::BoundingObject*> (node->GetData());
    if (boundingObject)
    {
      //delete item;
      m_TreeWidget->takeTopLevelItem(m_TreeWidget->indexOfTopLevelItem(item));
      m_ItemNodeMap.erase(m_ItemNodeMap.find(item));
      m_DataStorage->Remove(node);
    }
  }
}

void QmitkBoundingObjectWidget::RemoveAllItems()
{
  ItemNodeMapType::iterator it = m_ItemNodeMap.begin();

  while( it != m_ItemNodeMap.end() )
  {
    m_TreeWidget->takeTopLevelItem( m_TreeWidget->indexOfTopLevelItem(it->first) );
    m_ItemNodeMap.erase(m_ItemNodeMap.find(it->first));

    ++it;
  }

  m_BoundingObjectCounter = 1;
}


mitk::BoundingObject::Pointer QmitkBoundingObjectWidget::GetSelectedBoundingObject()
{
  mitk::BoundingObject* boundingObject;
  mitk::DataNode* node = this->GetSelectedBoundingObjectNode();

  if (node)
  {
    boundingObject = dynamic_cast<mitk::BoundingObject*> (node->GetData());
    if (boundingObject)
      return boundingObject;
  }
  return NULL;
}

void QmitkBoundingObjectWidget::SetDataStorage(mitk::DataStorage* dataStorage)
{
  m_DataStorage = dataStorage;
}

mitk::DataStorage* QmitkBoundingObjectWidget::GetDataStorage()
{
  return m_DataStorage;
}

void QmitkBoundingObjectWidget::OnDelButtonClicked()
{
  RemoveItem();
}

void QmitkBoundingObjectWidget::CreateBoundingObject(int type)
{

  //get cross position
  mitk::Point3D pos;
  mitk::RenderingManager::RenderWindowVector windows =  mitk::RenderingManager::GetInstance()->GetAllRegisteredRenderWindows();

  //hopefully we have the renderwindows in the "normal" order
  const mitk::PlaneGeometry *plane1 =
    mitk::BaseRenderer::GetInstance(windows.at(0))->GetSliceNavigationController()->GetCurrentPlaneGeometry();
  const mitk::PlaneGeometry *plane2 =
    mitk::BaseRenderer::GetInstance(windows.at(1))->GetSliceNavigationController()->GetCurrentPlaneGeometry();
  const mitk::PlaneGeometry *plane3 =
    mitk::BaseRenderer::GetInstance(windows.at(2))->GetSliceNavigationController()->GetCurrentPlaneGeometry();

  mitk::Line3D line;
  if ( (plane1 != NULL) && (plane2 != NULL)
    && (plane1->IntersectionLine( plane2, line )) )
  {
    if ( !((plane3 != NULL)
      && (plane3->IntersectionPoint( line, pos ))) )
    {
      return;
    }
  }

  if (type != 0)
  {
    mitk::BoundingObject::Pointer boundingObject;
    QString name;
    name.setNum(m_BoundingObjectCounter);

    switch (type-1)
    {
    case CUBOID:
      boundingObject = mitk::Cuboid::New();
      name.prepend("Cube_");
      break;
    case CONE:
      boundingObject = mitk::Cone::New();
      name.prepend("Cone_");
      break;
    case ELLIPSOID:
      boundingObject = mitk::Ellipsoid::New();
      name.prepend("Ellipse_");
      break;
    case CYLINDER:
      boundingObject = mitk::Cylinder::New();
      name.prepend("Cylinder_");
      break;
    default:
      return;
      break;
    }
    m_BoundingObjectCounter++;
    m_addComboBox->setCurrentIndex(0);

    // set initial size
    mitk::Vector3D size;
    size.Fill(10);
    boundingObject->GetGeometry()->SetSpacing( size );

    boundingObject->GetGeometry()->Translate(pos.GetVectorFromOrigin());
    boundingObject->GetTimeGeometry()->Update();

    //create node
    mitk::DataNode::Pointer node  = mitk::DataNode::New();
    node->SetData( boundingObject);
    node->SetProperty("name", mitk::StringProperty::New( name.toLocal8Bit().data()));
    node->SetProperty("color", mitk::ColorProperty::New(0.0, 0.0, 1.0));
    node->SetProperty("opacity", mitk::FloatProperty::New(0.7));
    node->SetProperty("bounding object", mitk::BoolProperty::New(true));
    node->SetProperty("helper object", mitk::BoolProperty::New(true));

    m_DataStorage->Add(node);

    mitk::RenderingManager::GetInstance()->RequestUpdateAll();

    emit BoundingObjectsChanged();

    AddItem(node);
  }
}

mitk::DataNode::Pointer QmitkBoundingObjectWidget::GetAllBoundingObjects()
{
  mitk::DataNode::Pointer boundingObjectGroupNode = mitk::DataNode::New();
  mitk::BoundingObjectGroup::Pointer boundingObjectGroup = mitk::BoundingObjectGroup::New();
  boundingObjectGroup->SetCSGMode(mitk::BoundingObjectGroup::Union);

  mitk::NodePredicateProperty::Pointer prop = mitk::NodePredicateProperty::New("bounding object", mitk::BoolProperty::New(true));
  mitk::DataStorage::SetOfObjects::ConstPointer allBO = m_DataStorage->GetSubset(prop);

  for (mitk::DataStorage::SetOfObjects::const_iterator it = allBO->begin(); it != allBO->end(); ++it)
  {
    mitk::DataNode::Pointer node = *it;
    mitk::BoundingObject::Pointer boundingObject = dynamic_cast<mitk::BoundingObject*> (node->GetData());
    if (boundingObject)
      boundingObjectGroup->AddBoundingObject(boundingObject);
  }

  boundingObjectGroupNode->SetData(boundingObjectGroup);

  if (boundingObjectGroup->GetCount() >0)
    return boundingObjectGroupNode;

  return NULL;
}

mitk::DataNode::Pointer QmitkBoundingObjectWidget::GetSelectedBoundingObjectNode()
{
  QList <QTreeWidgetItem*> selectedItems = m_TreeWidget->selectedItems();
  if (selectedItems.size() <1)
    return NULL;

  QTreeWidgetItem* item = selectedItems.first();
  mitk::DataNode* node = m_ItemNodeMap.find(item)->second;

  return node;
}

void QmitkBoundingObjectWidget::OnBoundingObjectModified(const itk::EventObject&)
{
  emit BoundingObjectsChanged();
}
