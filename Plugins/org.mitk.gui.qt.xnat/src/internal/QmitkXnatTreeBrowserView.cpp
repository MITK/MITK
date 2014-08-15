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

#include "QmitkXnatTreeBrowserView.h"

// Qmitk
#include "QmitkXnatObjectEditorInput.h"
#include "QmitkXnatEditor.h"
#include "org_mitk_gui_qt_xnatinterface_Activator.h"

// Blueberry
#include <berryIWorkbenchPage.h>

// CTK XNAT Core
#include "ctkXnatFile.h"

const std::string QmitkXnatTreeBrowserView::VIEW_ID = "org.mitk.views.xnat.treebrowser";

QmitkXnatTreeBrowserView::QmitkXnatTreeBrowserView():
  m_Session(0),
  m_TreeModel(new ctkXnatTreeModel())
{
}

QmitkXnatTreeBrowserView::~QmitkXnatTreeBrowserView()
{
  delete m_TreeModel;
}

void QmitkXnatTreeBrowserView::SetFocus()
{
}

void QmitkXnatTreeBrowserView::CreateQtPartControl( QWidget *parent )
{
  // Get the XNAT Session from Activator
  m_Session = mitk::org_mitk_gui_qt_xnatinterface_Activator::GetXnatSessionManager()->GetXnatSession();

  if(m_Session == 0)
  {
    m_Controls.labelError->setText("Please check the Preferences of XNAT. Maybe they are not ok.");
    m_Controls.labelError->setStyleSheet("QLabel { color: red; }");
    return;
  }

  // Create GUI widgets from the Qt Designer's .ui file
  m_Controls.setupUi( parent );
  m_Controls.treeView->header()->hide();

  m_SelectionProvider = new berry::QtSelectionProvider();
  this->SetSelectionProvider();
  m_Controls.treeView->setSelectionMode(QAbstractItemView::SingleSelection);

  connect( m_Controls.treeView, SIGNAL(activated(const QModelIndex&)), this, SLOT(OnActivatedNode(const QModelIndex&)) );

  UpdateSession();
}

void QmitkXnatTreeBrowserView::OnActivatedNode(const QModelIndex& index)
{
  if (!index.isValid()) return;

  berry::IWorkbenchPage::Pointer page = GetSite()->GetPage();
  QmitkXnatObjectEditorInput::Pointer oPtr = QmitkXnatObjectEditorInput::New( index.data(Qt::UserRole).value<ctkXnatObject*>() );
  berry::IEditorInput::Pointer editorInput( oPtr );
  berry::IEditorPart::Pointer reuseEditor = page->FindEditor(editorInput);

  if(reuseEditor)
  {
    // Just set it activ
    page->Activate(reuseEditor);
  }
  else
  {
    std::vector<berry::IEditorReference::Pointer> editors =
      page->FindEditors(berry::IEditorInput::Pointer(0), QmitkXnatEditor::EDITOR_ID, berry::IWorkbenchPage::MATCH_ID);

    if (editors.empty())
    {
      // No XnatEditor is currently open, create a new one
      ctkXnatFile* file = dynamic_cast<ctkXnatFile*>(oPtr->GetXnatObject());
      if(file != NULL)
      {
        // If a file is activated take the parent and open a new editor
        QmitkXnatObjectEditorInput::Pointer oPtr2 = QmitkXnatObjectEditorInput::New( file->parent() );
        berry::IEditorInput::Pointer editorInput2( oPtr2 );
        page->OpenEditor(editorInput2, QmitkXnatEditor::EDITOR_ID);
      }
      else
      {
        page->OpenEditor(editorInput, QmitkXnatEditor::EDITOR_ID);
      }
    }
    else
    {
      // Reuse an existing editor
      reuseEditor = editors.front()->GetEditor(true);
      page->ReuseEditor(reuseEditor.Cast<berry::IReusableEditor>(), editorInput);
      page->Activate(reuseEditor);
    }
  }
}

void QmitkXnatTreeBrowserView::SetSelectionProvider()
{
  GetSite()->SetSelectionProvider(m_SelectionProvider);
}

void QmitkXnatTreeBrowserView::UpdateSession()
{
  delete m_TreeModel;
  m_TreeModel = new ctkXnatTreeModel();
  m_Controls.treeView->setModel(m_TreeModel);
  m_Controls.treeView->reset();

  // Get the XNAT Session from Activator
  m_Session = mitk::org_mitk_gui_qt_xnatinterface_Activator::GetXnatSessionManager()->GetXnatSession();

  if(m_Session != NULL)
  {
    connect( this->m_Session, SIGNAL(destroyed()), this, SLOT(UpdateSession()) );

    // Fill model and show in the GUI
    m_Controls.treeView->setModel(m_TreeModel);
    m_TreeModel->addDataModel(m_Session->dataModel());
    m_Controls.treeView->reset();
    m_SelectionProvider->SetItemSelectionModel(m_Controls.treeView->selectionModel());
  }
}
