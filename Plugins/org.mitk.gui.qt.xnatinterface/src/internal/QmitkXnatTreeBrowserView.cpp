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

// Qmitk
#include "QmitkXnatTreeBrowserView.h"
#include "QmitkXnatObjectEditorInput.h"
#include "QmitkXnatEditor.h"
#include "org_mitk_gui_qt_xnatinterface_Activator.h"

// Standard
#include <iostream>
#include <string>

// Blueberry
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>
#include <berryIWorkbenchPage.h>
#include <berryIEditorInput.h>

// Qt
#include <QMessageBox>
#include <QTreeView>
#include <QAbstractItemModel>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QString>
#include <QBrush>
#include <QListView>

const std::string QmitkXnatTreeBrowserView::VIEW_ID = "org.mitk.views.qmitkxnattreebrowserview";

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
  // create GUI widgets from the Qt Designer's .ui file
  m_Controls.setupUi( parent );
  m_Controls.treeView->setModel(m_TreeModel);

  m_SelectionProvider = new berry::QtSelectionProvider();
  m_SelectionProvider->SetItemSelectionModel(m_Controls.treeView->selectionModel());
  this->SetSelectionProvider();
  m_Controls.treeView->setSelectionMode(QAbstractItemView::SingleSelection);

  connect( m_Controls.treeView, SIGNAL(activated(const QModelIndex&)), this, SLOT(OnActivatedNode(const QModelIndex&)) );
  connect( m_Controls.treeView, SIGNAL(clicked(const QModelIndex&)), this, SLOT(DoFetchMore(const QModelIndex&)) );

  // get the XNAT Session from Activator
  m_Session = mitk::org_mitk_gui_qt_xnatinterface_Activator::GetXnatConnectionManager()->GetXnatConnection();

  // fill model and show in the GUI
  m_TreeModel->addDataModel(m_Session->dataModel());
  m_Controls.treeView->reset();
}

void QmitkXnatTreeBrowserView::OnSelectionChanged( berry::IWorkbenchPart::Pointer /*source*/,
                                      const QList<mitk::DataNode::Pointer>& nodes )
{
}

void QmitkXnatTreeBrowserView::OnActivatedNode(const QModelIndex& index)
{
  if (!index.isValid()) return;

  if ( m_TreeModel->canFetchMore(index) )
  {
    m_TreeModel->fetchMore(index);
  }

  berry::IWorkbenchPage::Pointer page = GetSite()->GetPage();
  QmitkXnatObjectEditorInput::Pointer oPtr = QmitkXnatObjectEditorInput::New( index.data(Qt::EditRole).value<ctkXnatObject*>() );
  berry::IEditorInput::Pointer editorInput( oPtr );
  berry::IEditorPart::Pointer reuseEditor = page->FindEditor(editorInput);

  if(reuseEditor)
  {
    //just set it activ
    reuseEditor.Cast<QmitkXnatEditor>()->UpdateList();
    page->Activate(reuseEditor);
  }
  else
  {
    reuseEditor = page->GetActiveEditor();

    if( reuseEditor.IsNotNull() && page->GetReference(reuseEditor)->GetId() == QmitkXnatEditor::EDITOR_ID )
    {
      page->ReuseEditor(reuseEditor.Cast<berry::IReusableEditor>(), editorInput);
      reuseEditor.Cast<QmitkXnatEditor>()->UpdateList();
      page->Activate(reuseEditor);
    }
    else
    {
      std::vector<berry::IEditorReference::Pointer> editors =
        page->FindEditors(berry::IEditorInput::Pointer(0), QmitkXnatEditor::EDITOR_ID, berry::IWorkbenchPage::MATCH_ID);

      if (editors.empty())
      {
        // no XnatEditor is currently open, create a new one
        page->OpenEditor(editorInput, QmitkXnatEditor::EDITOR_ID);
      }
      else
      {
        // reuse an existing editor
        reuseEditor = editors.front()->GetEditor(true);
        page->ReuseEditor(reuseEditor.Cast<berry::IReusableEditor>(), editorInput);
        reuseEditor.Cast<QmitkXnatEditor>()->UpdateList();
        page->Activate(reuseEditor);
      }
    }
  }
}

void QmitkXnatTreeBrowserView::SetSelectionProvider()
{
  GetSite()->SetSelectionProvider(m_SelectionProvider);
}

void QmitkXnatTreeBrowserView::DoFetchMore(const QModelIndex &index)
{
  if (!index.isValid()) return;

  if ( m_TreeModel->canFetchMore(index) )
  {
    m_TreeModel->fetchMore(index);
  }

  std::vector<berry::IEditorReference::Pointer> editors =
    GetSite()->GetPage()->FindEditors(berry::IEditorInput::Pointer(0), QmitkXnatEditor::EDITOR_ID, berry::IWorkbenchPage::MATCH_ID);

  if (!editors.empty())
  {
    berry::IEditorPart::Pointer editor = editors.front()->GetEditor(true);
    editor.Cast<QmitkXnatEditor>()->UpdateList();
  }
}
