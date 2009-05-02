/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2008-10-30 12:33:31 +0100 (Do, 30 Okt 2008) $
Version:   $Revision: 15606 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "QmitkToolReferenceDataSelectionBox.h"
#include "QmitkDataStorageComboBox.h"
//#include "QmitkPropertyListPopup.h"

#include "mitkNodePredicateProperty.h"
#include "mitkNodePredicateDataType.h"
#include "mitkNodePredicateDimension.h"
#include "mitkNodePredicateAND.h"
#include "mitkNodePredicateOR.h"
#include "mitkNodePredicateNOT.h"

#include "mitkRenderingManager.h"

QmitkToolReferenceDataSelectionBox::QmitkToolReferenceDataSelectionBox(QWidget* parent)
:QWidget(parent),
m_SelfCall(false),
m_DisplayMode(ListDataIfAnyToolMatches ),
m_ToolGroupsForFiltering("default")
{
  m_ToolManager = mitk::ToolManager::New();

  m_Layout = new QVBoxLayout( this );
  this->setLayout( m_Layout );

  m_ReferenceDataSelectionBox = new QmitkDataStorageComboBox( this );
  m_Layout->addWidget(m_ReferenceDataSelectionBox);

  connect( m_ReferenceDataSelectionBox, SIGNAL(OnSelectionChanged(const mitk::DataTreeNode*)),
    this, SLOT(OnReferenceDataSelected(const mitk::DataTreeNode*)) );

  m_ToolManager->ReferenceDataChanged += mitk::MessageDelegate<QmitkToolReferenceDataSelectionBox>( this, &QmitkToolReferenceDataSelectionBox::OnToolManagerReferenceDataModified );
}

QmitkToolReferenceDataSelectionBox::~QmitkToolReferenceDataSelectionBox()
{
  m_ToolManager->ReferenceDataChanged -= mitk::MessageDelegate<QmitkToolReferenceDataSelectionBox>( this, &QmitkToolReferenceDataSelectionBox::OnToolManagerReferenceDataModified );
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

void QmitkToolReferenceDataSelectionBox::OnReferenceDataSelected(const mitk::DataTreeNode* selectedNode)
{
  emit ReferenceNodeSelected(selectedNode);

  m_SelfCall = true;
  m_ToolManager->SetReferenceData( const_cast< mitk::DataTreeNode*>(selectedNode)); // maybe NULL
  m_SelfCall = false;

  EnsureOnlyReferenceImageIsVisibile();
}

void QmitkToolReferenceDataSelectionBox::EnsureOnlyReferenceImageIsVisibile()
{
  mitk::DataTreeNode* selectedNode = m_ToolManager->GetReferenceData(0);

  mitk::DataStorage::SetOfObjects::ConstPointer allImageNodes = GetAllPossibleReferenceImages();
  for ( mitk::DataStorage::SetOfObjects::const_iterator nodeIter = allImageNodes->begin();
    nodeIter != allImageNodes->end();
    ++nodeIter )
  {
    mitk::DataTreeNode* currentNode = (*nodeIter).GetPointer();

    currentNode->SetVisibility( currentNode == selectedNode ); // only the selected one is visible, everything else is invisible
  }

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();

}

void QmitkToolReferenceDataSelectionBox::OnToolManagerReferenceDataModified()
{
  if (m_SelfCall) return;

  const mitk::DataTreeNode* node = m_ToolManager->GetReferenceData(0);
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
          m_Predicates.push_back( mitk::NodePredicateOR::New( completePredicate, tool->GetReferenceDataPreference() ).GetPointer() );
        }
        else
        {
          m_Predicates.push_back( mitk::NodePredicateAND::New( completePredicate, tool->GetReferenceDataPreference() ).GetPointer() );
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
  mitk::DataStorage* dataStorage = mitk::DataStorage::GetInstance();

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
    mitk::DataTreeNode* node = (*objectIter).GetPointer();
    resultVector.push_back( node );
  }


  mitk::DataStorage::SetOfObjects::ConstPointer sceneImages = mitk::DataStorage::GetInstance()->GetSubset( completePredicate );
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

