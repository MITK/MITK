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
#include "mitkPointSetWriter.h"
#include "mitkVesselGraphFileWriter.h"
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
  mitk::GlobalInteraction* globalInteraction = dynamic_cast<mitk::GlobalInteraction*> (mitk::EventMapper::GetGlobalStateMachine());
  if (globalInteraction)
  {
    mitk::FocusManager* fm = globalInteraction->GetFocusManager();
    QmitkFocusChangeCommand::Pointer fcc = QmitkFocusChangeCommand::New();
    fcc->SetQmitkDataManagerControls(this);
    mitk::FocusEvent fe;

    fm->AddObserver(fe,fcc);
  }
  if(mitk::ChiliPlugin::IsPlugin()==false)
    m_SaveToLightBox->hide();
}

void QmitkDataManagerControls::destroy()
{
}

// init the combobox with all
void QmitkDataManagerControls::UpdateRendererCombo()
{
  mitk::RenderWindow* focusedRenderWindow = NULL;

  mitk::GlobalInteraction* globalInteraction =
    dynamic_cast<mitk::GlobalInteraction*> (mitk::EventMapper::GetGlobalStateMachine());
  if (globalInteraction)
  {
    mitk::FocusManager* fm = globalInteraction->GetFocusManager();
    mitk::BaseRenderer::ConstPointer br = fm->GetFocused();
    if (br.IsNotNull())
    {
      focusedRenderWindow = br->GetRenderWindow();
    }
  }

  m_RenderWindowCombo->clear();
  const mitk::RenderWindow::RenderWindowSet rws = mitk::RenderWindow::GetInstances();
  for (mitk::RenderWindow::RenderWindowSet::const_iterator iter = rws.begin();iter != rws.end();iter++)
  {
    if ((*iter)->GetName())
    {
      m_RenderWindowCombo->insertItem(QString((*iter)->GetName()),0);
      //        if ((*iter) == focusedRenderWindow) {
      //          m_RenderWindowCombo->setCurrentItem(0);
      //        }
    }

  }
  if (focusedRenderWindow)
  {
    m_RenderWindowCombo->setCurrentText(focusedRenderWindow->GetName());
    // m_RenderWindowCombo->activated(focusedRenderWindow->name());
  }
  else
  {
    m_RenderWindowCombo->setCurrentText("no focused window");
  }
  // TODO: new interface
  //  m_RendererPropertiesView->SetRenderWindow(m_RenderWindowCombo->currentText());
}
void QmitkDataManagerControls::SetDataTreeIterator(mitk::DataTreeIteratorBase* it)
{
  if (it == NULL) return;
  m_NodePropertiesView->SetPropertyList(NULL);
  m_RendererPropertiesView->SetPropertyList(NULL);

  while (m_DataTreeView->firstChild())
  {
    delete m_DataTreeView->firstChild();
  }
  m_DataTreeIterator = it;
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
          mitk::Image::Pointer image = dynamic_cast<mitk::Image*>(data.GetPointer());
          if(image.IsNotNull())
          {
            typedef itk::Image<int,3> ImageType;
            CommonFunctionality::SaveImage< ImageType >(image);
          }
          mitk::PointSet::Pointer pointset = dynamic_cast<mitk::PointSet*>(data.GetPointer());
          if(pointset.IsNotNull())
          {
            selectedItemsName = itksys::SystemTools::GetFilenameWithoutExtension(selectedItemsName);
            selectedItemsName += ".mps";
            QString fileName = QFileDialog::getSaveFileName(QString(selectedItemsName.c_str()),"MITK Point-Sets (*.mps)");
            if (fileName != NULL )
            {
              mitk::PointSetWriter::Pointer writer = mitk::PointSetWriter::New();
              writer->SetInput( pointset );
              writer->SetFileName( fileName.ascii() );
              writer->Update();
            }
          }
          mitk::Surface::Pointer surface = dynamic_cast<mitk::Surface*>(data.GetPointer());
          if(surface.IsNotNull())
          {
            CommonFunctionality::SaveSurface(surface, "SurfaceModel.stl");
          }
          mitk::UndirectedVesselGraphData::Pointer uvg = dynamic_cast<mitk::UndirectedVesselGraphData*>(data.GetPointer());
          if (uvg.IsNotNull())
          {
            QString fileName = QFileDialog::getSaveFileName(QString("VesselGraph.uvg"),"MITK VesselGraph (*.uvg)");
            if (fileName != NULL ) 
            {
              mitk::VesselGraphFileWriter<Undirected>::Pointer writer = mitk::VesselGraphFileWriter<Undirected>::New();
              writer->SetInput( uvg );
              writer->SetFileName( fileName.ascii() );
              writer->Update();
            }
          }
          mitk::DirectedVesselGraphData::Pointer dvg = dynamic_cast<mitk::DirectedVesselGraphData*>(data.GetPointer());
          if (dvg.IsNotNull())
          {
            QString fileName = QFileDialog::getSaveFileName(QString("VesselGraph.dvg"),"MITK VesselGraph (*.dvg)");
            if (fileName != NULL ) 
            {
              mitk::VesselGraphFileWriter<Directed>::Pointer writer = mitk::VesselGraphFileWriter<Directed>::New();
              writer->SetInput( dvg );
              writer->SetFileName( fileName.ascii() );
              writer->Update();
            }
          }
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
  m_TransferFunctionWidget->SetDataTreeNode(dtvi->GetDataTreeNode());  m_NodePropertiesView->SetPropertyList(dtvi->GetDataTreeNode()->GetPropertyList());

  mitk::RenderWindow* focusedRenderWindow = NULL;

  mitk::GlobalInteraction* globalInteraction =
    dynamic_cast<mitk::GlobalInteraction*> (mitk::EventMapper::GetGlobalStateMachine());
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
    m_RendererPropertiesView->SetPropertyList(dtvi->GetDataTreeNode()->GetPropertyList(focusedRenderWindow->GetRenderer()));
  }
}
