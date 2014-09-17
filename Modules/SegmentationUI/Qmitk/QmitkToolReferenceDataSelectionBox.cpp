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

#include "QmitkToolReferenceDataSelectionBox.h"
#include "QmitkDataStorageComboBox.h"
//#include "QmitkPropertyListPopup.h"

#include "mitkNodePredicateProperty.h"
#include "mitkNodePredicateDataType.h"
#include "mitkNodePredicateDimension.h"
#include "mitkNodePredicateAnd.h"
#include "mitkNodePredicateOr.h"
#include "mitkNodePredicateNot.h"

#include "mitkRenderingManager.h"

#include "mitkToolManagerProvider.h"

QmitkToolReferenceDataSelectionBox::QmitkToolReferenceDataSelectionBox(QWidget* parent)
:QWidget(parent),
m_SelfCall(false),
m_DisplayMode(ListDataIfAnyToolMatches ),
m_ToolGroupsForFiltering("default")
{
  m_ToolManager = mitk::ToolManagerProvider::GetInstance()->GetToolManager();

  m_Layout = new QVBoxLayout( this );
  this->setLayout( m_Layout );

  m_ReferenceDataSelectionBox = new QmitkDataStorageComboBox( this );
  m_Layout->addWidget(m_ReferenceDataSelectionBox);

  connect( m_ReferenceDataSelectionBox, SIGNAL(OnSelectionChanged(const mitk::DataNode*)),
    this, SLOT(OnReferenceDataSelected(const mitk::DataNode*)) );

  m_ToolManager->ReferenceDataChanged += mitk::MessageDelegate<QmitkToolReferenceDataSelectionBox>( this, &QmitkToolReferenceDataSelectionBox::OnToolManagerReferenceDataModified );
}

QmitkToolReferenceDataSelectionBox::~QmitkToolReferenceDataSelectionBox()
{
  m_ToolManager->ReferenceDataChanged -= mitk::MessageDelegate<QmitkToolReferenceDataSelectionBox>( this, &QmitkToolReferenceDataSelectionBox::OnToolManagerReferenceDataModified );
}

mitk::DataStorage* QmitkToolReferenceDataSelectionBox::GetDataStorage()
{
  return m_ToolManager->GetDataStorage();
}

void QmitkToolReferenceDataSelectionBox::SetDataStorage(mitk::DataStorage& storage)
{
  m_ToolManager->SetDataStorage(storage);
}

void QmitkToolReferenceDataSelectionBox::Initialize(mitk::DataStorage* storage )
{
  m_ReferenceDataSelectionBox->SetDataStorage( storage );

  UpdateDataDisplay();
}


mitk::ToolManager* QmitkToolReferenceDataSelectionBox::GetToolManager()
{
  return m_ToolManager;
}

void QmitkToolReferenceDataSelectionBox::SetToolManager(mitk::ToolManager& newManager) // no NULL pointer allowed here, a manager is required
{
  m_ToolManager->ReferenceDataChanged -= mitk::MessageDelegate<QmitkToolReferenceDataSelectionBox>( this, &QmitkToolReferenceDataSelectionBox::OnToolManagerReferenceDataModified );

  m_ToolManager = &newManager;

  m_ToolManager->ReferenceDataChanged += mitk::MessageDelegate<QmitkToolReferenceDataSelectionBox>( this, &QmitkToolReferenceDataSelectionBox::OnToolManagerReferenceDataModified );

  UpdateDataDisplay();
}

void QmitkToolReferenceDataSelectionBox::UpdateDataDisplay()
{
  m_ReferenceDataSelectionBox->SetPredicate( GetAllPossibleReferenceImagesPredicate().GetPointer() );
  EnsureOnlyReferenceImageIsVisibile();
}

void QmitkToolReferenceDataSelectionBox::OnReferenceDataSelected(const mitk::DataNode* selectedNode)
{
  emit ReferenceNodeSelected(selectedNode);

  m_SelfCall = true;
  m_ToolManager->SetReferenceData( const_cast< mitk::DataNode*>(selectedNode)); // maybe NULL
  m_SelfCall = false;

  EnsureOnlyReferenceImageIsVisibile();
}

void QmitkToolReferenceDataSelectionBox::EnsureOnlyReferenceImageIsVisibile()
{
  mitk::DataNode* selectedNode = m_ToolManager->GetReferenceData(0);

  mitk::DataStorage::SetOfObjects::ConstPointer allImageNodes = GetAllPossibleReferenceImages();
  for ( mitk::DataStorage::SetOfObjects::const_iterator nodeIter = allImageNodes->begin();
    nodeIter != allImageNodes->end();
    ++nodeIter )
  {
    mitk::DataNode* currentNode = (*nodeIter).GetPointer();

    currentNode->SetVisibility( currentNode == selectedNode ); // only the selected one is visible, everything else is invisible
  }

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();

}

void QmitkToolReferenceDataSelectionBox::OnToolManagerReferenceDataModified()
{
  if (m_SelfCall) return;

  const mitk::DataNode* node = m_ToolManager->GetReferenceData(0);
  emit ReferenceNodeSelected(node);

  UpdateDataDisplay();
}

mitk::NodePredicateBase::ConstPointer QmitkToolReferenceDataSelectionBox::GetAllPossibleReferenceImagesPredicate()
{
  /**
  * Build up predicate:
  *  - ask each tool that is displayed for a predicate (indicating the type of data that this tool will work with)
  *  - connect all predicates using AND or OR, depending on the parameter m_DisplayMode (ListDataIfAllToolsMatch or ListDataIfAnyToolMatches)
  *    \sa SetDisplayMode
  */

  std::vector< mitk::NodePredicateBase::ConstPointer > m_Predicates;
  m_Predicates.clear();
  mitk::NodePredicateBase::ConstPointer completePredicate = NULL;

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
      if (completePredicate)
      {
        if ( m_DisplayMode == ListDataIfAnyToolMatches )
        {
          m_Predicates.push_back( mitk::NodePredicateOr::New( completePredicate, tool->GetReferenceDataPreference() ).GetPointer() );
        }
        else
        {
          m_Predicates.push_back( mitk::NodePredicateAnd::New( completePredicate, tool->GetReferenceDataPreference() ).GetPointer() );
        }
        completePredicate = m_Predicates.back();
      }
      else
      {
        completePredicate = tool->GetReferenceDataPreference();
      }
    }
  }

  return completePredicate;
}

mitk::DataStorage::SetOfObjects::ConstPointer QmitkToolReferenceDataSelectionBox::GetAllPossibleReferenceImages()
{
  mitk::DataStorage* dataStorage = m_ToolManager->GetDataStorage();
  if (!dataStorage)
  {
    return mitk::DataStorage::SetOfObjects::New().GetPointer();
  }

  mitk::NodePredicateBase::ConstPointer completePredicate = GetAllPossibleReferenceImagesPredicate();

  mitk::DataStorage::SetOfObjects::ConstPointer allObjects;

  /**
  *  display everything matching the predicate
  */
  if (completePredicate.IsNotNull())
  {
    allObjects = dataStorage->GetSubset( completePredicate );
  }
  else
  {
    allObjects = dataStorage->GetAll();
  }

  mitk::ToolManager::DataVectorType resultVector;

  for ( mitk::DataStorage::SetOfObjects::const_iterator objectIter = allObjects->begin();
    objectIter != allObjects->end();
    ++objectIter )
  {
    mitk::DataNode* node = (*objectIter).GetPointer();
    resultVector.push_back( node );
  }


  mitk::DataStorage::SetOfObjects::ConstPointer sceneImages = dataStorage->GetSubset( completePredicate );
  return sceneImages;
}

void QmitkToolReferenceDataSelectionBox::SetToolGroupsForFiltering(const std::string& groups)
{
  m_ToolGroupsForFiltering = groups;
  UpdateDataDisplay();
}

void QmitkToolReferenceDataSelectionBox::SetDisplayMode( QmitkToolReferenceDataSelectionBox::DisplayMode mode )
{
  if (m_DisplayMode != mode)
  {
    m_DisplayMode = mode;
    UpdateDataDisplay();
  }
}

