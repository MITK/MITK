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
#include "QmitkXnatSimpleSearchView.h"

// Standard
#include <iostream>
#include <string>

// Blueberry
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>

// Qt
#include <QMessageBox>
#include <QTreeView>
#include <QAbstractItemModel>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QString>
#include <QBrush>
#include <QListView>

// ctkXnatCore
#include "ctkXnatProject.h"


enum MatchingObject
{
  EMPTY,
  PROJECT,
  SUBJECT,
  EXPERIMENT
};

const std::string QmitkXnatSimpleSearchView::VIEW_ID = "org.mitk.views.qmitkxnatsimplesearchview";

QmitkXnatSimpleSearchView::QmitkXnatSimpleSearchView() :
  m_Session(0),
  m_TreeModel(new ctkXnatTreeModel())
{
}

QmitkXnatSimpleSearchView::~QmitkXnatSimpleSearchView()
{
  if ( m_Session ) delete m_Session;
  delete m_TreeModel;
}

void QmitkXnatSimpleSearchView::SetFocus()
{
  m_Controls.buttonStartSearch->setFocus();
}

void QmitkXnatSimpleSearchView::CreateQtPartControl( QWidget *parent )
{
  // create GUI widgets from the Qt Designer's .ui file
  m_Controls.setupUi( parent );
  m_Controls.treeView->setModel(m_TreeModel);

  connect( m_Controls.buttonStartSearch, SIGNAL(clicked()), this, SLOT(StartSearch()) );
}

void QmitkXnatSimpleSearchView::OnSelectionChanged( berry::IWorkbenchPart::Pointer /*source*/,
                                      const QList<mitk::DataNode::Pointer>& nodes )
{
  // iterate all selected objects, adjust warning visibility
  //foreach( mitk::DataNode::Pointer node, nodes )
  //{
  //  if( node.IsNotNull() && dynamic_cast<mitk::Image*>(node->GetData()) )
  //  {
  //    m_Controls.labelWarning->setVisible( false );
  //    m_Controls.buttonStartSearch->setEnabled( true );
  //    return;
  //  }
  //}

  //m_Controls.labelWarning->setVisible( true );
  //m_Controls.buttonStartSearch->setEnabled( false );
}

void QmitkXnatSimpleSearchView::StartSearch()
{
  int type;

  switch(m_Controls.objectComboBox->currentIndex())
  {
  case MatchingObject::PROJECT:
    type = MatchingObject::PROJECT;
    break;
  case MatchingObject::SUBJECT:
    type = MatchingObject::SUBJECT;
    break;
  case MatchingObject::EXPERIMENT:
    type = MatchingObject::EXPERIMENT;
    break;
  default:
    type = MatchingObject::EMPTY;
    MITK_INFO << "You did something weird or chose nothing!";
    break;
  }

  //m_TreeModel->setMatchingObject( type );
  //m_TreeModel->setSearchTerm( m_Controls.termLineEdit->text() );
  //m_TreeModel->setIsStartOfSearch( true );

  // create ctkXnatConnection
  //m_Connection = m_ConnectionFactory->makeConnection(m_Controls.inHostAddress->text(), m_Controls.inUser->text(), m_Controls.inPassword->text());
  //ctkXnatServer* server = m_Connection->server();

  //m_TreeModel->addServer(server);
  //m_Controls.treeView->reset();
}
