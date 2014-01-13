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
  m_Profile(new ctkXnatLoginProfile()),
  m_TreeModel(new ctkXnatTreeModel())
{
}

QmitkXnatTreeBrowserView::~QmitkXnatTreeBrowserView()
{
  delete m_Profile;
  if ( m_Session )
  {
    m_Session->close();
    delete m_Session;
  }
  delete m_TreeModel;
}

void QmitkXnatTreeBrowserView::SetFocus()
{
  m_Controls.buttonStartBrowser->setFocus();
}

void QmitkXnatTreeBrowserView::CreateQtPartControl( QWidget *parent )
{
  // create GUI widgets from the Qt Designer's .ui file
  m_Controls.setupUi( parent );
  m_Controls.treeView->setModel(m_TreeModel);

  m_SelectionProvider = new berry::QtSelectionProvider();
  m_SelectionProvider->SetItemSelectionModel(m_Controls.treeView->selectionModel());
  SetSelectionProvider();
  m_Controls.treeView->setSelectionMode(QAbstractItemView::SingleSelection);

  connect( m_Controls.buttonStartBrowser, SIGNAL(clicked()), this, SLOT(StartBrowser()) );

  connect( m_Controls.treeView, SIGNAL(activated(const QModelIndex&)), this, SLOT(OnActivatedNode(const QModelIndex&)) );
  connect( m_Controls.treeView, SIGNAL(clicked(const QModelIndex&)), this, SLOT(DoFetchMore(const QModelIndex&)) );
}

void QmitkXnatTreeBrowserView::OnSelectionChanged( berry::IWorkbenchPart::Pointer /*source*/,
                                      const QList<mitk::DataNode::Pointer>& nodes )
{
  // iterate all selected objects, adjust warning visibility
  //foreach( mitk::DataNode::Pointer node, nodes )
  //{
  //  if( node.IsNotNull() && dynamic_cast<mitk::Image*>(node->GetData()) )
  //  {
  //    m_Controls.labelWarning->setVisible( false );
  //    m_Controls.buttonStartBrowser->setEnabled( true );
  //    return;
  //  }
  //}

  //m_Controls.labelWarning->setVisible( true );
  //m_Controls.buttonStartBrowser->setEnabled( false );
}

void QmitkXnatTreeBrowserView::StartBrowser()
{
  if ( m_Session )
  {
    return;
    //delete m_Session;
    //m_Session = 0;
    //QModelIndex index = m_TreeModel->index(1,1,QModelIndex());
    //m_TreeModel->removeAllRows(index);
  }
  else
  {
    CheckUserInput();

    // fill profile
    m_Profile->setName(QString("localhost"));
    m_Profile->setServerUrl(m_Controls.inHostAddress->text());
    m_Profile->setUserName(m_Controls.inUser->text());
    m_Profile->setPassword(m_Controls.inPassword->text());
    m_Profile->setDefault(true);

    // create ctkXnatSession with the profile
    m_Session = new ctkXnatSession(*m_Profile);
    m_Session->open();

    // fill model and show in the GUI
    m_TreeModel->addDataModel(m_Session->dataModel());
    m_Controls.treeView->reset();
  }
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

void QmitkXnatTreeBrowserView::CheckUserInput()
{
  // Validate user input
  if ( m_Controls.inHostAddress->text().isEmpty() || m_Controls.inUser->text().isEmpty() || m_Controls.inPassword->text().isEmpty() ) {
    if ( m_Controls.inHostAddress->text().isEmpty() )
    {
      MITK_INFO << "No host address!";
      m_Controls.inHostAddress->setStyleSheet("QLineEdit{ background-color: rgb(255,0,0) }");
    }

    if ( m_Controls.inUser->text().isEmpty() ) {
      MITK_INFO << "No user !";
      m_Controls.inUser->setStyleSheet("QLineEdit{ background-color: rgb(255,0,0) }");
    }

    if ( m_Controls.inPassword->text().isEmpty() ) {
      MITK_INFO << "No password!";
      m_Controls.inPassword->setStyleSheet("QLineEdit{ background-color: rgb(255,0,0) }");
    }
  }
  else
  {
    m_Controls.inHostAddress->setStyleSheet("QLineEdit{ background-color: rgb(255,255,255) }");
    m_Controls.inUser->setStyleSheet("QLineEdit{ background-color: rgb(255,255,255) }");
    m_Controls.inPassword->setStyleSheet("QLineEdit{ background-color: rgb(255,255,255) }");
  }

  // Regular Expression for uri
  QRegExp uriregex("^(https?)://([a-zA-Z0-9\\.]+):([0-9]+)(/[^ /]+)*$");

  // Validate address
  if ( !uriregex.exactMatch(m_Controls.inHostAddress->text()) )
  {
    MITK_INFO << m_Controls.inHostAddress->text().toStdString();
    MITK_INFO << "Host address not valid";
    m_Controls.inHostAddress->setStyleSheet("QLineEdit{ background-color: rgb(255,0,0) }");
  }
  else
  {
    m_Controls.inHostAddress->setStyleSheet("QLineEdit{ background-color: rgb(255,255,255) }");
  }
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

ctkXnatTreeModel* QmitkXnatTreeBrowserView::GetTreeModel()
{
  return m_TreeModel;
}
