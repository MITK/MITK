/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/
#include <utility>
#include "mitkProperties.h"
#include "mitkEventMapper.h"
#include "mitkFocusManager.h"
#include "mitkPointSetWriter.h"

#include <qfiledialog.h>
#include "ipPic.h"
#include "ipFunc.h"

void QmitkDataManagerControls::init() { 
  while (m_DataTreeView->columns() > 0 ) {
	m_DataTreeView->removeColumn(0);
  }
  m_DataTreeView->addColumn( "Name" );
  m_DataTreeView->addColumn( "NodeType" );
  m_DataTreeView->addColumn( "RefCount");
  m_RemoveButton->setEnabled(false);
  mitk::GlobalInteraction* globalInteraction = dynamic_cast<mitk::GlobalInteraction*> (mitk::EventMapper::GetGlobalStateMachine());
  if (globalInteraction) {
    mitk::FocusManager* fm = globalInteraction->GetFocusManager();
    QmitkFocusChangeCommand::Pointer fcc = QmitkFocusChangeCommand::New();
    fcc->SetQmitkDataManagerControls(this); 
    mitk::FocusEvent fe; 

    fm->AddObserver(fe,fcc);
  }
}

// init the combobox with all   
void QmitkDataManagerControls::UpdateRendererCombo() {
  mitk::RenderWindow* focusedRenderWindow = NULL;  

  mitk::GlobalInteraction* globalInteraction = 
    dynamic_cast<mitk::GlobalInteraction*> (mitk::EventMapper::GetGlobalStateMachine());
  if (globalInteraction) {
    mitk::FocusManager* fm = globalInteraction->GetFocusManager();
    mitk::BaseRenderer::ConstPointer br = fm->GetFocused();
    if (br.IsNotNull()) {
      focusedRenderWindow = br->GetRenderWindow();
    }
  }

 m_RenderWindowCombo->clear(); 
  const mitk::RenderWindow::RenderWindowSet rws = mitk::RenderWindow::GetInstances();
  for (mitk::RenderWindow::RenderWindowSet::const_iterator iter = rws.begin();iter != rws.end();iter++) {
    if ((*iter)->GetName()) {
      m_RenderWindowCombo->insertItem(QString((*iter)->GetName()),0);
//        if ((*iter) == focusedRenderWindow) {
//          m_RenderWindowCombo->setCurrentItem(0);
//        }
    }
 
  }
  if (focusedRenderWindow) {
    m_RenderWindowCombo->setCurrentText(focusedRenderWindow->GetName()); 
    // m_RenderWindowCombo->activated(focusedRenderWindow->name());
  } else {
    m_RenderWindowCombo->setCurrentText("no focused window");
  }
  m_RendererPropertiesView->SetRenderWindow(m_RenderWindowCombo->currentText()); 
}
void QmitkDataManagerControls::SetDataTreeIterator(mitk::DataTreeIterator * it)
{
    while (m_DataTreeView->firstChild()) {
	    delete m_DataTreeView->firstChild();
     }
    m_DataTreeIterator = it->clone();
    mitk::DataTreeIterator* tempIt = m_DataTreeIterator->clone();

  if (tempIt->hasNext()) 
  {
    tempIt->next();
    QmitkDataTreeViewItem * rootItem = new QmitkDataTreeViewItem(m_DataTreeView, "Loaded Data", "root", tempIt->clone());
  }
  delete tempIt;
  UpdateRendererCombo();
}

void QmitkDataManagerControls::RemoveButtonClicked()
{
  QmitkDataTreeViewItem *selected = dynamic_cast<QmitkDataTreeViewItem*>(m_DataTreeView->selectedItem());
  if (selected == NULL) {
  } else {
    mitk::DataTreeIterator* selectedIterator = selected->GetDataTreeIterator();
    assert(selectedIterator != NULL);
    selectedIterator->remove();
    delete selected;
  } 
}


void QmitkDataManagerControls::SaveButton_clicked()
{
  QmitkDataTreeViewItem *selected = dynamic_cast<QmitkDataTreeViewItem*>(m_DataTreeView->selectedItem());
  if (selected != NULL) {
    std::string m_SelectedItemsName = std::string(selected->text(0).ascii());
  
    mitk::DataTreeIterator* selectedIterator = selected->GetDataTreeIterator();
//    mitk::DataTreeIterator* selectedIterator = ((mitk::DataTree *) m_DataTreeIterator->getTree())->GetNext("name", new mitk::StringProperty( m_SelectedItemsName  ));
    if (selectedIterator != NULL)
    {
      mitk::DataTreeNode* node = selectedIterator->get();
      std::cout << "2" << std::endl;
      if (node != NULL ){
        mitk::BaseData::Pointer data=node->GetData();

        if (data.IsNotNull())
        {
          mitk::Image::Pointer image = dynamic_cast<mitk::Image*>(data.GetPointer());
          if(image.IsNotNull())
          {
            ipPicDescriptor * picImage = image->GetPic();

            picImage = ipFuncRefl(picImage,3);

            QString fileName = QFileDialog::getSaveFileName(QString(m_SelectedItemsName),"DKFZ Pic (*.seq *.pic *.pic.gz *.seq.gz *.dcm)");
            if (fileName == NULL ) 
            {
              fileName = QString(m_SelectedItemsName.c_str());
            }

            ipPicPut((char *)fileName.ascii(), picImage);
          }
          mitk::PointSet::Pointer pointset = dynamic_cast<mitk::PointSet*>(data.GetPointer());
          if(pointset.IsNotNull())
          {
            QString fileName = QFileDialog::getSaveFileName(QString(m_SelectedItemsName),"DKFZ PointSetFile (.mps)");
            if (fileName != NULL ) 
            {
              mitk::PointSetWriter::Pointer writer = mitk::PointSetWriter::New();
              writer->SetInput( pointset );
              writer->SetFileName( fileName.latin1() );
              writer->Update();
            }
          }
        }
      }
    } else {
      std::cout << "logical error" << std::endl;
    }
  }
}
