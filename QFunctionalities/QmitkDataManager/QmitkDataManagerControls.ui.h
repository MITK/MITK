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


/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/

#include <QmitkCommonFunctionality.h>
#include "QmitkPACSSaveDialog.h"
#include <utility>

#include <mitkFocusManager.h>
#include <mitkRenderingManager.h>
#include <mitkAffineInteractor.h>
#include <QmitkSystemInfo.h>
#include <mitkCoreObjectFactory.h>
#include <mitkPACSPlugin.h>

#include <mitkBaseRenderer.h>
#include <vtkRenderWindow.h>

#include <itkImage.h>

#include <itksys/SystemTools.hxx>

#include <qfiledialog.h>
#include <qmessagebox.h>
#include <qtextedit.h>
#include <qapplication.h>

void QmitkDataManagerControls::destroy()
{
}

// init the combobox with all
void QmitkDataManagerControls::UpdateRendererCombo()
{
  // refill the combo boxes
  m_RenderWindowCombo->clear();
  m_RenderWindowComboMulti->clear();
  
  for(mitk::BaseRenderer::BaseRendererMapType::iterator mapit = mitk::BaseRenderer::baseRendererMap.begin(); 
      mapit != mitk::BaseRenderer::baseRendererMap.end(); mapit++)
  {
    if((*mapit).second->GetName())
    {
      std::string winName((*mapit).second->GetName());
      //  winName.erase(0,winName.find("::")+2);
      m_RenderWindowCombo->insertItem(winName.c_str());
      m_RenderWindowComboMulti->insertItem(winName.c_str());
    }
  }
  
  // try to select focused RenderWindow
  vtkRenderWindow* focusedRenderWindow = NULL;

  mitk::FocusManager* fm =
    mitk::GlobalInteraction::GetInstance()->GetFocusManager();
  mitk::BaseRenderer::ConstPointer br = fm->GetFocused();
  if (br.IsNotNull())
  {
    focusedRenderWindow = br->GetRenderWindow();
  }

  if (focusedRenderWindow)
  {
    
    m_RenderWindowCombo->setCurrentText(mitk::BaseRenderer::GetInstance(focusedRenderWindow)->GetName());
    m_RenderWindowComboMulti->setCurrentText(mitk::BaseRenderer::GetInstance(focusedRenderWindow)->GetName());
  }
  else
  {
    m_RenderWindowCombo->setCurrentText("no focused window");
    m_RenderWindowComboMulti->setCurrentText("no focused window");
  }
  RendererChange();
  RendererChangeMulti();
}

void QmitkDataManagerControls::SetDataTreeIterator(mitk::DataTreeIteratorBase* it)
{
  if (it == NULL) return;
  m_NodePropertiesView->SetPropertyList(NULL);

  while (m_DataTreeView->firstChild())
  {
    delete m_DataTreeView->firstChild();
  }
  m_DataTreeIterator = it;
  InitMultiMode(NULL);
  mitk::DataTreeIteratorClone tempIt = m_DataTreeIterator;

  if (!tempIt->IsAtEnd())
  {
    new QmitkDataTreeViewItem(m_DataTreeView, "Loaded Data", "root", tempIt.GetPointer());
    ++tempIt;
  }
  UpdateRendererCombo();
}

void QmitkDataManagerControls::RemoveButtonClicked()
{
  QmitkDataTreeViewItem *selected = dynamic_cast<QmitkDataTreeViewItem*>(m_DataTreeView->selectedItem());
  if (selected) {
    mitk::DataTreeIteratorClone selectedIterator = selected->GetDataTreeIterator();
    assert(selectedIterator.IsNotNull());

    switch(QMessageBox::question(this, tr("DataManager"), tr("Do you really want to delete the item '").append(selected->text(0)).append("' ?"),
          QMessageBox::Yes | QMessageBox::Default, QMessageBox::No, QMessageBox::Cancel | QMessageBox::Escape)) 
    {
      case QMessageBox::Yes: //Remove the item from view and tree
        delete selected;
        selectedIterator->Remove();
        mitk::RenderingManager::GetInstance()->RequestUpdateAll();
        break;

      case QMessageBox::No: break;
      case QMessageBox::Cancel: break;
    }
  }
}

void QmitkDataManagerControls::SaveButton_clicked()
{
  QmitkDataTreeViewItem *selected = dynamic_cast<QmitkDataTreeViewItem*>(m_DataTreeView->selectedItem());
  if (selected != NULL)
  {
    std::string selectedItemsName = std::string(selected->text(0).ascii());

    mitk::DataTreeIteratorBase* selectedIterator = selected->GetDataTreeIterator();
    if (selectedIterator != NULL)
    {
      mitk::DataTreeNode* node = selectedIterator->Get();
      if (node != NULL )
      {
        mitk::BaseData::Pointer data=node->GetData();

        if (data.IsNotNull())
        {
          CommonFunctionality::SaveBaseData( data.GetPointer(), selectedItemsName.c_str() );
        }
      }
    }
    else
    {
      std::cout << "logical error" << std::endl;
    }
  }
}

void QmitkDataManagerControls::SaveToPACSButton_clicked()
{
  QmitkDataTreeViewItem *selected = dynamic_cast<QmitkDataTreeViewItem*>(m_DataTreeView->selectedItem());
  if (selected != NULL)
  {
    std::string selectedItemsName = std::string(selected->text(0).ascii());

    mitk::DataTreeIteratorBase* selectedIterator = selected->GetDataTreeIterator();
    if (selectedIterator != NULL)
    {
      mitk::DataTreeNode* node = selectedIterator->Get();
      if (node != NULL )
      {
        std::vector<mitk::DataTreeNode*> nodesToSave;
        nodesToSave.clear();
        nodesToSave.push_back( node );

        QmitkPACSSaveDialog dialog(this);
        dialog.SetDataTreeNodesToSave( nodesToSave );
        dialog.setAllowSeveralObjectsIntoOneSeries(true); // just cannot select more than one object here :-(
        dialog.exec();

        if ( dialog.result() == QDialog::Accepted )
        {
          std::cout << "PACS export dialog reported success." << std::endl;
        }
        else
        {
          std::cout << "PACS export dialog cancelled by user." << std::endl;
        }
      }
    }
    else
    {
      std::cout << "logical error" << std::endl;
    }
  }
}


void QmitkDataManagerControls::ReInitButton_clicked()
{
  QmitkDataTreeViewItem *selected = dynamic_cast<QmitkDataTreeViewItem*>(m_DataTreeView->selectedItem());
  if (selected != NULL)
  {
    mitk::DataTreeIteratorClone selectedIterator = selected->GetDataTreeIterator();
    assert(selectedIterator.IsNotNull());
    mitk::DataTreeNode* node = selectedIterator->Get();
    if (node != NULL )
    {
      mitk::BaseData::Pointer basedata = node->GetData();
      if (basedata.IsNotNull())
      {
        mitk::RenderingManager::GetInstance()->InitializeViews(
          basedata->GetTimeSlicedGeometry(), mitk::RenderingManager::REQUEST_UPDATE_ALL, true );
        mitk::RenderingManager::GetInstance()->RequestUpdateAll();
      }
    }
  }
}

void QmitkDataManagerControls::TreeSelectionChanged( QListViewItem * item )
{
  QmitkDataTreeViewItem* dtvi = dynamic_cast<QmitkDataTreeViewItem*>(item);
  if (dtvi != NULL) {
    assert(dtvi->GetDataTreeNode().IsNotNull());
    m_NodePropertiesView->SetPropertyList(dtvi->GetDataTreeNode()->GetPropertyList());
    RendererChange();
    AffineInteraction_clicked();
  } 
  bool buttonState = ! (dtvi == NULL || dtvi->GetDataTreeNode() == m_DataTreeIterator->Get());
  m_SaveButton->setEnabled(buttonState);
  m_SaveToPACS->setEnabled(buttonState);
  m_RemoveButton->setEnabled(buttonState);    
}

void QmitkDataManagerControls::RenderWindowSelected( int id )
{
  std::cout << "RenderWindowSelected() called" << std::endl;
  QmitkDataTreeViewItem *selected = dynamic_cast<QmitkDataTreeViewItem*>(m_DataTreeView->selectedItem());
  if (selected != NULL)
  {
    int selectedItem = id;
    int itemNumber = -1;
    
    mitk::BaseRenderer::BaseRendererMapType::iterator mapit;  
    for(mapit = mitk::BaseRenderer::baseRendererMap.begin(); 
        mapit != mitk::BaseRenderer::baseRendererMap.end(); mapit++, itemNumber++)
    {
      ++itemNumber;
      if(itemNumber==selectedItem)
        break;
    }
    if(itemNumber==selectedItem)
    {
      // FIX: m_RendererPropertiesView->SetPropertyList(node->GetPropertyList((*iter)->GetRenderer()));
    }
  }
}

void QmitkDataManagerControls::InitMultiMode(mitk::BaseRenderer* /*renderer*/)
{
  std::vector<std::string> propNames;
  propNames.push_back("name");
  propNames.push_back("visible");
  propNames.push_back("color");
  propNames.push_back("opacity");


  CommonFunctionality::DataTreeIteratorVector treeNodes;
  mitk::DataTreeIteratorClone it = m_DataTreeIterator;
  while (!it->IsAtEnd())
  {
    treeNodes.push_back(it);
    ++it;
  }
  m_MultiNodePropertiesView->SetMultiMode(propNames,treeNodes);
}

void QmitkDataManagerControls::RenderWindowCB_toggled( bool isOn )
{
  m_RenderWindowCombo->setEnabled(isOn);
  RendererChange();
}

void QmitkDataManagerControls::RenderWindowCBMulti_toggled( bool isOn )
{
  m_RenderWindowComboMulti->setEnabled(isOn);
  RendererChangeMulti();
}

void QmitkDataManagerControls::RenderWindowCombo_activated( int )
{
  //
  std::cout << "Combo activated" << std::endl;
  RendererChange();
}

void QmitkDataManagerControls::RenderWindowComboMulti_activated( int )
{
  //
  std::cout << "MultiCombo activated" << std::endl;
  RendererChangeMulti();

}

void QmitkDataManagerControls::RendererChange()
{
  QmitkDataTreeViewItem *selectedItem = dynamic_cast<QmitkDataTreeViewItem*>(m_DataTreeView->selectedItem());
  if (selectedItem != NULL)
  {
    if ( mitk::DataTreeNode* selectedNode = selectedItem->GetDataTreeIterator()->Get())
    {
      if  (m_RenderWindowCombo->isEnabled())
      {
        QString rendererName = m_RenderWindowCombo->currentText();
        if(rendererName.isEmpty() == false)
        {
          vtkRenderWindow* renWin =  mitk::BaseRenderer::GetRenderWindowByName(rendererName.ascii());
                
          if (renWin)
          {
            m_NodePropertiesView->SetPropertyList(selectedNode->GetPropertyList(mitk::BaseRenderer::GetInstance(renWin)) );
          }
        }
      }
      else
      {
        m_NodePropertiesView->SetPropertyList(selectedNode->GetPropertyList());
      }
    }
  }
}

void QmitkDataManagerControls::RendererChangeMulti()
{
  std::vector<std::string> propNames;
  propNames.push_back("name");
  propNames.push_back("visible");
  propNames.push_back("color");
  propNames.push_back("opacity");
  CommonFunctionality::DataTreeIteratorVector treeNodes;
  mitk::DataTreeIteratorClone it = m_DataTreeIterator;
  while (!it->IsAtEnd())
  {
    treeNodes.push_back(it);
    ++it;
  }
  mitk::BaseRenderer* renderer = NULL;
  if  (m_RenderWindowComboMulti->isEnabled())
  {
    vtkRenderWindow* renWin =  mitk::BaseRenderer::GetRenderWindowByName(m_RenderWindowComboMulti->currentText().ascii());
    if (renWin)
    {
      renderer = mitk::BaseRenderer::GetInstance(renWin);
    }
  }
  m_MultiNodePropertiesView->SetMultiMode(propNames, treeNodes, renderer);
}


void QmitkDataManagerControls::LoadButton_clicked()
{
  mitk::DataTreeIteratorClone selectedIterator = NULL;

  QmitkDataTreeViewItem *selected = dynamic_cast<QmitkDataTreeViewItem*>(m_DataTreeView->selectedItem());
  if (selected != NULL)
  {
    selectedIterator = selected->GetDataTreeIterator();
  }
  else
  {
    selectedIterator = m_DataTreeIterator;
  }

  QStringList fileNames = QFileDialog::getOpenFileNames(mitk::CoreObjectFactory::GetInstance()->GetFileExtensions(), NULL);
  for ( QStringList::Iterator it = fileNames.begin(); it != fileNames.end(); ++it )
  {
    FileOpen((*it).ascii(), selectedIterator.GetPointer());
  }
}


void QmitkDataManagerControls::AffineInteraction_clicked()
{
  QmitkDataTreeViewItem *selected = dynamic_cast<QmitkDataTreeViewItem*>(m_DataTreeView->selectedItem());
  if (selected != NULL)
  {
    TurnInteractionOff();
    if(m_AffineInteraction->isOn())
    {
      if(selected->GetDataTreeNode()->GetData()!=NULL)
      {
        m_AffineInteractionNode = selected->GetDataTreeNode();
        m_PreviousInteractor = m_AffineInteractionNode->GetInteractor();     
        m_AffineInteractor = mitk::AffineInteractor::New("AffineInteractions ctrl-drag", m_AffineInteractionNode);
        m_AffineInteractionNode->SetInteractor(m_AffineInteractor);

        mitk::GlobalInteraction::GetInstance()->AddInteractor(m_AffineInteractor);
      }
    }
  }
}


void QmitkDataManagerControls::Info_clicked()
{
  QmitkDataTreeViewItem *selected = dynamic_cast<QmitkDataTreeViewItem*>(m_DataTreeView->selectedItem());
  if( (selected != NULL) && (selected->GetDataTreeNode()->GetData() != NULL) )
  {
    QmitkSystemInfo *info = new QmitkSystemInfo(this, "QmitkSystemInfo::DataInfo");
    info->m_TabWidget->setCurrentPage(1);
    info->m_DataNodeName->setText(selected->text(0));
    ::itk::OStringStream itkOutput;
    selected->GetDataTreeNode()->GetData()->Print(itkOutput);
    if(selected->GetDataTreeNode()->GetInteractor() != NULL)
      selected->GetDataTreeNode()->GetInteractor()->Print(itkOutput);
    info->m_teOutputPane->append("--------------------DataInfo--------------------");
    info->m_teOutputPane->append(selected->text(0));
    info->m_teOutputPane->append(itkOutput.str().c_str());
    info->m_teOutputPane->append(selected->text(0));
    info->m_teOutputPane->append("----------------End of DataInfo-----------------");
    info->show();
  }
}

void QmitkDataManagerControls::FileOpen( const char * fileName, mitk::DataTreeIteratorBase* parentIterator )
{
  mitk::DataTreeNodeFactory::Pointer factory = mitk::DataTreeNodeFactory::New();

  try
  {
    factory->SetFileName( fileName );

    QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );

    factory->Update();

    for ( unsigned int i = 0 ; i < factory->GetNumberOfOutputs( ); ++i )
    {
      mitk::DataTreeNode::Pointer node = factory->GetOutput( i );
      if ( ( node.IsNotNull() ) && ( node->GetData() != NULL ) )
      {
        parentIterator->Add( node );
      }
    }
  }
  catch ( itk::ExceptionObject & ex )
  {
    itkGenericOutputMacro( << "Exception during file open: " << ex );
  }

  QApplication::restoreOverrideCursor();
}


void QmitkDataManagerControls::TurnInteractionOff()
{
  if(m_AffineInteractionNode.IsNotNull())
  {
    // did we add an interactor before? If yes, remove it.
    if(m_AffineInteractor.IsNotNull())
      mitk::GlobalInteraction::GetInstance()->RemoveInteractor(m_AffineInteractor);
    // does the m_AffineInteractionNode still contain the interactor we set on it?
    if(m_AffineInteractionNode->GetInteractor() == m_AffineInteractor)
    {
      // set the previous one
      m_AffineInteractionNode->SetInteractor(m_PreviousInteractor);
      // activate it, if any
      if(m_PreviousInteractor.IsNotNull())
        mitk::GlobalInteraction::GetInstance()->AddInteractor(m_PreviousInteractor);
    }
    m_AffineInteractionNode = NULL;
    m_PreviousInteractor = NULL;
    m_AffineInteractor = NULL;
  }
}


void QmitkDataManagerControls::GlobalReInit_clicked()
{
  if(m_DataTreeIterator.IsNotNull())
  {
    mitk::RenderingManager::GetInstance()->InitializeViews(
      m_DataTreeIterator.GetPointer() );
  }
}


void QmitkDataManagerControls::init()
{
#ifndef MBI_INTERNAL
  //m_AffineInteraction->hide();
#endif
    
  mitk::PACSPlugin::PACSPluginCapability pacsCapabilities = mitk::PACSPlugin::GetInstance()->GetPluginCapabilities();
  if ( pacsCapabilities.IsPACSFunctional && pacsCapabilities.HasSaveCapability )
  {
    m_SaveToPACS->show();
  }
  else
  {
    m_SaveToPACS->hide();
  }
}

