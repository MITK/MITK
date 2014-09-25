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
  m_TreeModel(new ctkXnatTreeModel()),
  m_Tracker(0)
{
}

QmitkXnatTreeBrowserView::~QmitkXnatTreeBrowserView()
{
  delete m_TreeModel;
  delete m_Tracker;
}

void QmitkXnatTreeBrowserView::SetFocus()
{
}

void QmitkXnatTreeBrowserView::CreateQtPartControl( QWidget *parent )
{
  // Create GUI widgets from the Qt Designer's .ui file
  m_Controls.setupUi( parent );
  m_Controls.treeView->setModel(m_TreeModel);
  m_Controls.treeView->header()->hide();
  m_Controls.labelError->setText("Please use the 'Connect' button in the Preferences.");
  m_Controls.labelError->setStyleSheet("QLabel { color: red; }");

  m_SelectionProvider = new berry::QtSelectionProvider();
  this->SetSelectionProvider();
  m_Controls.treeView->setSelectionMode(QAbstractItemView::SingleSelection);

  m_Tracker = new mitk::XnatSessionTracker(mitk::org_mitk_gui_qt_xnatinterface_Activator::GetXnatModuleContext());

  connect( m_Tracker, SIGNAL(AboutToBeClosed(ctkXnatSession*)), this, SLOT(CleanTreeModel(ctkXnatSession*)) );
  connect( m_Tracker, SIGNAL(Opened(ctkXnatSession*)), this, SLOT(UpdateSession(ctkXnatSession*)) );

  m_Tracker->Open();

  ctkXnatSession* session;

  try
  {
    session = mitk::org_mitk_gui_qt_xnatinterface_Activator::GetXnatModuleContext()->GetService(
      mitk::org_mitk_gui_qt_xnatinterface_Activator::GetXnatModuleContext()->GetServiceReference<ctkXnatSession>());
  }
  catch(std::invalid_argument)
  {
    session = 0;
  }

  if(session != 0)
  {
    m_Controls.labelError->setVisible(false);
  }
  else
  {
    m_Controls.labelError->setVisible(true);
  }

  connect( m_Controls.treeView, SIGNAL(activated(const QModelIndex&)), this, SLOT(OnActivatedNode(const QModelIndex&)) );
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

void QmitkXnatTreeBrowserView::UpdateSession(ctkXnatSession* session)
{
  if(session != 0 && session->isOpen())
  {
    m_Controls.labelError->setVisible(false);
    // Fill model and show in the GUI
    m_TreeModel->addDataModel(session->dataModel());
    m_Controls.treeView->reset();
    m_SelectionProvider->SetItemSelectionModel(m_Controls.treeView->selectionModel());
  }
}

void QmitkXnatTreeBrowserView::CleanTreeModel(ctkXnatSession* session)
{
  if(session != 0)
  {
    m_TreeModel->removeDataModel(session->dataModel());
    m_Controls.treeView->reset();
  }
}
