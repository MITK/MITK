/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/
#include <utility>
#include "mitkBoolProperty.h"
#include "mitkEventMapper.h"
#include "mitkFocusManager.h"
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
    if ((*iter)->name()) {
      m_RenderWindowCombo->insertItem(QString((*iter)->name()),0);
//        if ((*iter) == focusedRenderWindow) {
//          m_RenderWindowCombo->setCurrentItem(0);
//        }
    }
 
  }
  if (focusedRenderWindow) {
    m_RenderWindowCombo->setCurrentText(focusedRenderWindow->name()); 
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
    std::cout << "nothing selected" << std::endl;
  } else {
    mitk::DataTreeIterator* selectedIterator = selected->GetDataTreeIterator();
    assert(selectedIterator != NULL);
    selectedIterator->remove();
    delete selected;

  } 
}


