/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
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

#include "QmitkCommonFunctionality.h"
#include <utility>
#include "mitkProperties.h"
#include "mitkEventMapper.h"
#include "mitkFocusManager.h"
#include "itkImage.h"

#include <itksys/SystemTools.hxx>

#include <qfiledialog.h>
#include <qmessagebox.h>

#include "ipPic/ipPic.h"
#include "ipFunc/ipFunc.h"

#include "mitkChiliPlugin.h"
#include "mitkLightBoxResultImageWriter.h"

void QmitkDataManagerControls::init()
{
  m_DataTreeIterator = NULL;
  while (m_DataTreeView->columns() > 0 )
  {
    m_DataTreeView->removeColumn(0);
  }
  m_DataTreeView->addColumn( "Name" );
  m_DataTreeView->addColumn( "NodeType" );
  m_DataTreeView->addColumn( "RefCount");
  m_DataTreeView->setSortColumn(20);
  mitk::GlobalInteraction* globalInteraction =  mitk::GlobalInteraction::GetGlobalInteraction();
  if (globalInteraction)
  {
    mitk::FocusManager* fm = globalInteraction->GetFocusManager();
    QmitkFocusChangeCommand::Pointer fcc = QmitkFocusChangeCommand::New();
    fcc->SetQmitkDataManagerControls(this);
    mitk::FocusEvent fe;

    fm->AddObserver(fe,fcc);
  }
  if(mitk::ChiliPlugin::IsPlugin()==false)
  {
    m_SaveToLightBox->hide();
  }
}

void QmitkDataManagerControls::destroy()
{}

// init the combobox with all
void QmitkDataManagerControls::UpdateRendererCombo()
{
  // refill the combo boxes
  m_RenderWindowCombo->clear();
  m_RenderWindowComboMulti->clear();
  const mitk::RenderWindow::RenderWindowSet rws = mitk::RenderWindow::GetInstances();
  for (mitk::RenderWindow::RenderWindowSet::const_iterator iter = rws.begin();iter != rws.end();iter++)
  {
    if ((*iter)->GetName())
    {
      std::string winName((*iter)->GetName());
      //  winName.erase(0,winName.find("::")+2);
      m_RenderWindowCombo->insertItem(winName.c_str());
      m_RenderWindowComboMulti->insertItem(winName.c_str());
    }
  }

  // try to select focused RenderWindow
  mitk::RenderWindow* focusedRenderWindow = NULL;
  mitk::GlobalInteraction* globalInteraction =  mitk::GlobalInteraction::GetGlobalInteraction();
  if (globalInteraction)
  {
    mitk::FocusManager* fm = globalInteraction->GetFocusManager();
    mitk::BaseRenderer::ConstPointer br = fm->GetFocused();
    if (br.IsNotNull())
    {
      focusedRenderWindow = br->GetRenderWindow();
    }
  }
  if (focusedRenderWindow)
  {
    m_RenderWindowCombo->setCurrentText(focusedRenderWindow->GetName());
    m_RenderWindowComboMulti->setCurrentText(focusedRenderWindow->GetName());
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
    QmitkDataTreeViewItem * rootItem = new QmitkDataTreeViewItem(m_DataTreeView, "Loaded Data", "root", tempIt.GetPointer());
    ++tempIt;
  }
  UpdateRendererCombo();
}

void QmitkDataManagerControls::RemoveButtonClicked()
{
  QmitkDataTreeViewItem *selected = dynamic_cast<QmitkDataTreeViewItem*>(m_DataTreeView->selectedItem());
  if (selected == NULL)
  {}
  else
  {
    mitk::DataTreeIteratorClone selectedIterator = selected->GetDataTreeIterator();
    assert(selectedIterator.IsNotNull());
    delete selected;
    selectedIterator->Remove();
    mitk::RenderWindow::UpdateAllInstances();
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
          CommonFunctionality::SaveBaseData( data.GetPointer(), selectedItemsName );
        }
      }
    }
    else
    {
      std::cout << "logical error" << std::endl;
    }
  }
}


void QmitkDataManagerControls::SaveLightBox_clicked()
{
  if(mitk::ChiliPlugin::IsPlugin()==false)
  {
    QMessageBox::critical( this, "Save Selected to LightBox",
                           "Saving to LightBox not possible:\n"
                           "This is not a plugin!! ... or the plugin is not correctly initialized!",
                           QMessageBox::Cancel|QMessageBox::Default|QMessageBox::Escape,QMessageBox::NoButton );
    return;
  }

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
          mitk::Image::Pointer image = dynamic_cast<mitk::Image*>(data.GetPointer());
          if(image.IsNotNull())
          {
            mitk::LightBoxResultImageWriter::Pointer lbwriter =  mitk::LightBoxResultImageWriter::New();
            lbwriter->SetInputByNode(node);
            if(lbwriter->SetSourceByTreeSearch(selectedIterator)==false)
            {
              QMessageBox::critical( this, "Save Selected to LightBox",
                                     "Saving to LightBox not possible:\n"
                                     "Unable to find parent image that came from Chili.", QMessageBox::Cancel|QMessageBox::Default|QMessageBox::Escape,QMessageBox::NoButton );
              return;
            }
            lbwriter->SetLightBoxToCurrentLightBox();
            lbwriter->Write();
          }
        }
      }
    }
  }
}


void QmitkDataManagerControls::m_ReInitButton_clicked()
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
      emit InitializeStandardViews( basedata );
      mitk::RenderWindow::UpdateAllInstances();
    }
  }
}


void QmitkDataManagerControls::TreeSelectionChanged( QListViewItem * item )
{
  QmitkDataTreeViewItem* dtvi = dynamic_cast<QmitkDataTreeViewItem*>(item);
  assert(dtvi != NULL);
  assert(dtvi->GetDataTreeNode().IsNotNull());
  m_TransferFunctionWidget->SetDataTreeNode(dtvi->GetDataTreeNode());
  m_NodePropertiesView->SetPropertyList(dtvi->GetDataTreeNode()->GetPropertyList());
  RendererChange();
}

void QmitkDataManagerControls::RenderWindowSelected( int id )
{
  std::cout << "RenderWindowSelected() called" << std::endl;
  QmitkDataTreeViewItem *selected = dynamic_cast<QmitkDataTreeViewItem*>(m_DataTreeView->selectedItem());
  if (selected != NULL)
  {
    mitk::DataTreeNode* node = selected->GetDataTreeIterator()->Get();
    int selectedItem = id;
    int itemNumber = -1;
    const mitk::RenderWindow::RenderWindowSet rws = mitk::RenderWindow::GetInstances();
    mitk::RenderWindow::RenderWindowSet::const_iterator iter;
    for (iter = rws.begin();iter != rws.end();++iter)
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

void QmitkDataManagerControls::InitMultiMode(mitk::BaseRenderer* renderer)
{
  std::vector<std::string> propNames;
  propNames.push_back("name");
  propNames.push_back("visible");
  propNames.push_back("color");

  CommonFunctionality::DataTreeIteratorVector treeNodes;
  mitk::DataTreeIteratorClone it = m_DataTreeIterator;
  while (!it->IsAtEnd())
  {
    treeNodes.push_back(it);
    ++it;
  }
  std::cout << "Test clicked!" << std::endl;
  m_MultiNodePropertiesView->SetMultiMode(propNames,treeNodes);
}


void QmitkDataManagerControls::m_RenderWindowCB_toggled( bool isOn )
{
  m_RenderWindowCombo->setEnabled(isOn);
  RendererChange();
}


void QmitkDataManagerControls::m_RenderWindowCBMulti_toggled( bool isOn )
{
  m_RenderWindowComboMulti->setEnabled(isOn);
RendererChangeMulti();
}


void QmitkDataManagerControls::m_RenderWindowCombo_activated( int )
{
  //
  std::cout << "Combo activated" << std::endl;
  RendererChange();
}


void QmitkDataManagerControls::m_RenderWindowComboMulti_activated( int )
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
        const mitk::RenderWindow* renWin =  mitk::RenderWindow::GetByName(m_RenderWindowCombo->currentText());
        if (renWin)
        {
          m_NodePropertiesView->SetPropertyList(selectedNode->GetPropertyList(renWin->GetRenderer()));
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
  if  (m_RenderWindowComboMulti->isEnabled())
  {
    std::vector<std::string> propNames;
    propNames.push_back("name");
    propNames.push_back("visible");
    propNames.push_back("color");

    CommonFunctionality::DataTreeIteratorVector treeNodes;
    mitk::DataTreeIteratorClone it = m_DataTreeIterator;
    while (!it->IsAtEnd())
    {
      treeNodes.push_back(it);
      ++it;
    }
    mitk::BaseRenderer* renderer = NULL;
    const mitk::RenderWindow* renWin =  mitk::RenderWindow::GetByName(m_RenderWindowComboMulti->currentText());
    if (renWin)
    {
      renderer = renWin->GetRenderer();
    }
    m_MultiNodePropertiesView->SetMultiMode(propNames, treeNodes, renderer);
  }
}
