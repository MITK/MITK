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

#include "QmitkToolWorkingDataSelectionBox.h"
#include "mitkToolManagerProvider.h"

QmitkToolWorkingDataSelectionBox::QmitkToolWorkingDataSelectionBox(QWidget* parent)
:QListWidget(parent),
m_SelfCall(false),
m_LastSelectedReferenceData(NULL),
m_ToolGroupsForFiltering("default"),
m_DisplayOnlyDerivedNodes(true)
{
  m_ToolManager = mitk::ToolManagerProvider::GetInstance()->GetToolManager(); // this widget should be placeable from designer so it can't take other than the defaul parameters

  QListWidget::setSelectionMode( QListWidget::MultiSelection );
  QListWidget::setDragDropMode(QListWidget::InternalMove);

  connect( this, SIGNAL(itemSelectionChanged()), this, SLOT(OnWorkingDataSelectionChanged()) );

  m_ToolManager->ReferenceDataChanged += mitk::MessageDelegate<QmitkToolWorkingDataSelectionBox>( this, &QmitkToolWorkingDataSelectionBox::OnToolManagerReferenceDataModified );
  m_ToolManager->WorkingDataChanged += mitk::MessageDelegate<QmitkToolWorkingDataSelectionBox>( this, &QmitkToolWorkingDataSelectionBox::OnToolManagerWorkingDataModified );
}

QmitkToolWorkingDataSelectionBox::~QmitkToolWorkingDataSelectionBox()
{

}

mitk::DataStorage* QmitkToolWorkingDataSelectionBox::GetDataStorage()
{
  return m_ToolManager->GetDataStorage();
}

void QmitkToolWorkingDataSelectionBox::SetDataStorage(mitk::DataStorage& storage)
{
  m_ToolManager->SetDataStorage(storage);
}


mitk::ToolManager* QmitkToolWorkingDataSelectionBox::GetToolManager()
{
  return m_ToolManager;
}

void QmitkToolWorkingDataSelectionBox::SetToolManager(mitk::ToolManager& newManager) // no NULL pointer allowed here, a manager is required
{
  m_ToolManager->ReferenceDataChanged -= mitk::MessageDelegate<QmitkToolWorkingDataSelectionBox>( this, &QmitkToolWorkingDataSelectionBox::OnToolManagerReferenceDataModified );
  m_ToolManager->WorkingDataChanged -= mitk::MessageDelegate<QmitkToolWorkingDataSelectionBox>( this, &QmitkToolWorkingDataSelectionBox::OnToolManagerWorkingDataModified );

  m_ToolManager = &newManager;

  m_ToolManager->ReferenceDataChanged += mitk::MessageDelegate<QmitkToolWorkingDataSelectionBox>( this, &QmitkToolWorkingDataSelectionBox::OnToolManagerReferenceDataModified );
  m_ToolManager->WorkingDataChanged += mitk::MessageDelegate<QmitkToolWorkingDataSelectionBox>( this, &QmitkToolWorkingDataSelectionBox::OnToolManagerWorkingDataModified );

  UpdateDataDisplay();
}

void QmitkToolWorkingDataSelectionBox::OnWorkingDataSelectionChanged()
{
  static mitk::ToolManager::DataVectorType previouslySelectedNodes;

  mitk::ToolManager::DataVectorType selection = this->GetSelectedNodes();

  previouslySelectedNodes = selection;

  if (selection.size() >0)
  {
    const mitk::DataNode* node = selection[0];
    emit WorkingNodeSelected(node);
  }
  else
  {
    emit WorkingNodeSelected(NULL);
  }

  m_SelfCall = true;
  m_ToolManager->SetWorkingData( selection ); // maybe empty
  m_SelfCall = false;
}

void QmitkToolWorkingDataSelectionBox::OnToolManagerWorkingDataModified()
{
  if (m_SelfCall) return;

  const mitk::DataNode* node = m_ToolManager->GetWorkingData(0);
  emit WorkingNodeSelected(node);

  UpdateDataDisplay();
}

void QmitkToolWorkingDataSelectionBox::OnToolManagerReferenceDataModified()
{
  if ( m_ToolManager->GetReferenceData(0) != m_LastSelectedReferenceData )
  {
    m_ToolManager->SetWorkingData(NULL);
    UpdateDataDisplay();

    m_LastSelectedReferenceData = m_ToolManager->GetReferenceData(0);
  }
}

void QmitkToolWorkingDataSelectionBox::UpdateDataDisplay()
{

  // clear all
  QListWidget::clear();
  m_Node.clear();

  // rebuild contents
  mitk::ToolManager::DataVectorType allObjects = GetAllNodes( false );
  for ( mitk::ToolManager::DataVectorType::const_iterator objectIter = allObjects.begin();
    objectIter != allObjects.end();
    ++objectIter)
  {
    mitk::DataNode* node = *objectIter;

    if (node) // delete this check when datastorage is really used
    {
      // get name and color
      std::string name = node->GetName();
      float rgb[3]; rgb[0] = 1.0; rgb[1] = 0.0; rgb[2] = 0.0;
      node->GetColor(rgb);
      QRgb qrgb = qRgb( (int)(rgb[0]*255.0), (int)(rgb[1]*255.0),  (int)(rgb[2]*255.0) );

      QPixmap pixmap(25,18);
      pixmap.fill(QColor(qrgb));

      // create a list item
      QListWidgetItem* newItem = new QListWidgetItem();
      QString qname = QString::fromLocal8Bit(name.c_str());

      //set name and color
      newItem->setText(qname);
      newItem->setIcon(QIcon(pixmap));

      this->addItem(newItem);

      m_Node.insert( std::make_pair( newItem, node ) );
    }
  }
}

mitk::ToolManager::DataVectorType QmitkToolWorkingDataSelectionBox::GetSelectedNodes()
{
  mitk::ToolManager::DataVectorType result;

  QList<QListWidgetItem *> items;
  for (int j=0; j<this->count(); j++)
  {
    if (this->item(j)->isSelected())
      items.append(this->item(j));
  }

  for (int i=0; i<items.size();++i)
  {

    QListWidgetItem* item = items.at(i);
    if (item)
    {
      ItemNodeMapType::iterator it = m_Node.find(item);
      if (it != m_Node.end())
      {
        mitk::DataNode::Pointer node =   it->second;
        if (node)
          result.push_back(node);
      }
    }
  }

  return result;
}

mitk::DataNode* QmitkToolWorkingDataSelectionBox::GetSelectedNode()
{
  QListWidgetItem* item = QListWidget::selectedItems().first();
  if (item)
  {
    ItemNodeMapType::iterator iter = m_Node.find(item);
    if ( iter != m_Node.end() )
    {
      return iter->second;
    }
  }

  return NULL;
}

mitk::ToolManager::DataVectorType QmitkToolWorkingDataSelectionBox::GetAllNodes( bool onlyDerivedFromOriginal )
{
  mitk::DataStorage* dataStorage = m_ToolManager->GetDataStorage();
  if (!dataStorage)
  {
    return mitk::ToolManager::DataVectorType();
  }

  /**
  * Build up predicate:
  *  - ask each tool that is displayed for a predicate (indicating the type of data that this tool will work with)
  *  - connect all predicates using AND or OR, depending on the parameter m_DisplayMode (ListDataIfAllToolsMatch or ListDataIfAnyToolMatches)
  *    \sa SetDisplayMode
  */

  std::vector< mitk::NodePredicateBase::ConstPointer > m_Predicates;
  mitk::NodePredicateBase::ConstPointer completePredicate = NULL;
  bool rebuildNeeded = true;
  if (rebuildNeeded)
  {
    m_Predicates.clear();

    const mitk::ToolManager::ToolVectorTypeConst allTools = m_ToolManager->GetTools();

    for ( mitk::ToolManager::ToolVectorTypeConst::const_iterator iter = allTools.begin();
      iter != allTools.end();
      ++iter )
    {
      const mitk::Tool* tool = *iter;

      if ( (m_ToolGroupsForFiltering.empty()) || ( m_ToolGroupsForFiltering.find( tool->GetGroup() ) != std::string::npos ) ||
        ( m_ToolGroupsForFiltering.find( tool->GetName() )  != std::string::npos )
        )
      {
        if (completePredicate.IsNotNull())
        {

          m_Predicates.push_back( mitk::NodePredicateOr::New( completePredicate, tool->GetWorkingDataPreference()).GetPointer() );

          completePredicate = m_Predicates.back();
        }
        else
        {
          completePredicate = tool->GetWorkingDataPreference();
        }
      }
    }
  }

  // TODO delete all m_Predicates
  mitk::DataStorage::SetOfObjects::ConstPointer allObjects;

  /**
  * Two modes here:
  *  - display only nodes below reference data from ToolManager (onlyDerivedFromOriginal == true)
  *  - display everything matching the predicate (else)
  */

  if ( onlyDerivedFromOriginal )
  {
    mitk::DataNode* sourceNode( m_ToolManager->GetReferenceData(0) );
    if (sourceNode)
    {
      allObjects = dataStorage->GetDerivations( sourceNode, completePredicate, false );
    }
    else
    {
      allObjects = mitk::DataStorage::SetOfObjects::New();
    }
  }
  else
  {
    if (completePredicate)
    {
      allObjects = dataStorage->GetSubset( completePredicate );
    }
    else
    {
      allObjects = dataStorage->GetAll();
    }
  }

  m_Predicates.clear();
  completePredicate = NULL;

  mitk::ToolManager::DataVectorType resultVector;

  for ( mitk::DataStorage::SetOfObjects::const_iterator objectIter = allObjects->begin();
    objectIter != allObjects->end();
    ++objectIter )
  {
    mitk::DataNode* node = (*objectIter).GetPointer();
    resultVector.push_back( node );
  }

  return resultVector;
}
