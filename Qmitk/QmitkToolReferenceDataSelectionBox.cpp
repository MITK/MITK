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

#include "QmitkToolReferenceDataSelectionBox.h"
#include "QmitkDataTreeComboBox.h"
#include "QmitkPropertyListPopup.h"

#include "mitkNodePredicateProperty.h"
#include "mitkNodePredicateDataType.h"
#include "mitkNodePredicateDimension.h"
#include "mitkNodePredicateAND.h"
#include "mitkNodePredicateOR.h"
#include "mitkNodePredicateNOT.h"

#include "mitkRenderingManager.h"

QmitkToolReferenceDataSelectionBox::QmitkToolReferenceDataSelectionBox(QWidget* parent, const char* name)
:QVBox(parent, name),
 m_SelfCall(false)
{
  m_ToolManager = mitk::ToolManager::New();

  m_ReferenceDataSelectionBox = new QmitkDataTreeComboBox( this );

  connect( m_ReferenceDataSelectionBox, SIGNAL(activated(const mitk::DataTreeFilter::Item*)),
           this, SLOT(OnReferenceDataSelected(const mitk::DataTreeFilter::Item*)) );

  m_ToolManager->ReferenceDataChanged.AddListener( this, &QmitkToolReferenceDataSelectionBox::OnToolManagerReferenceDataModified );
}

QmitkToolReferenceDataSelectionBox::~QmitkToolReferenceDataSelectionBox()
{
}

void QmitkToolReferenceDataSelectionBox::Initialize(mitk::DataTreeBase* tree )
{
  m_ReferenceDataSelectionBox->SetDataTree( tree );
  
  UpdateDataDisplay();
}
    
mitk::ToolManager* QmitkToolReferenceDataSelectionBox::GetToolManager()
{
  return m_ToolManager;
}

void QmitkToolReferenceDataSelectionBox::SetToolManager(mitk::ToolManager& newManager) // no NULL pointer allowed here, a manager is required
{
  m_ToolManager->ReferenceDataChanged.RemoveListener( this, &QmitkToolReferenceDataSelectionBox::OnToolManagerReferenceDataModified );

  m_ToolManager = &newManager;

  m_ToolManager->ReferenceDataChanged.AddListener( this, &QmitkToolReferenceDataSelectionBox::OnToolManagerReferenceDataModified );

  UpdateDataDisplay();
}
     
void QmitkToolReferenceDataSelectionBox::UpdateDataDisplay()
{ 
  m_ReferenceDataSelectionBox->GetFilter()->SetDataStorageResultset( GetAllPossibleReferenceImages() ); /// \todo Also forward the current selected. Perhaps wait for new combobox.
  if (! m_ReferenceDataSelectionBox->GetFilter()->GetSelectMostRecentItemMode() )
  {
    m_ReferenceDataSelectionBox->GetFilter()->SetSelectMostRecentItemMode( true ); // automagically select new items
  }
  EnsureOnlyReferenceImageIsVisibile();
}

void QmitkToolReferenceDataSelectionBox::OnReferenceDataSelected(const mitk::DataTreeFilter::Item* item)
{
  mitk::DataTreeNode* selectedNode = const_cast<mitk::DataTreeNode*>( item->GetNode() );

  emit ReferenceNodeSelected(selectedNode);

  m_SelfCall = true;
  m_ToolManager->SetReferenceData(selectedNode); // maybe NULL
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

mitk::DataStorage::SetOfObjects::ConstPointer QmitkToolReferenceDataSelectionBox::GetAllPossibleReferenceImages()
{
  // update reference images
  mitk::NodePredicateDataType images("Image");
  mitk::NodePredicateDimension dim3(3, 1);
  mitk::NodePredicateDimension dim4(4, 1);
  mitk::NodePredicateOR dimension( dim3, dim4 );
  mitk::NodePredicateAND image3D( images, dimension );

  mitk::NodePredicateProperty binary("binary", mitk::BoolProperty::New(true));
  mitk::NodePredicateNOT notBinary( binary );

  mitk::NodePredicateProperty segmentation("segmentation", mitk::BoolProperty::New(true));
  mitk::NodePredicateNOT notSegmentation( segmentation );
  
  mitk::NodePredicateProperty helper("helper object", mitk::BoolProperty::New(true));
  mitk::NodePredicateNOT notHelper( helper );
  
  mitk::NodePredicateAND imageColorful( notBinary, notSegmentation );
  
  mitk::NodePredicateAND imageColorfulNotHelper( imageColorful, notHelper );
  
  mitk::NodePredicateAND completePredicate( image3D, imageColorfulNotHelper );

  mitk::DataStorage::SetOfObjects::ConstPointer sceneImages = mitk::DataStorage::GetInstance()->GetSubset( completePredicate );
  return sceneImages;
}

