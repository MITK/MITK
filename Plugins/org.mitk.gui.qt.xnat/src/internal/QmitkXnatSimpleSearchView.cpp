/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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

const std::string QmitkXnatSimpleSearchView::VIEW_ID = "org.mitk.views.qmitkxnatsimplesearchview";

QmitkXnatSimpleSearchView::QmitkXnatSimpleSearchView() :
  m_Session(0),
  m_TreeModel(new ctkXnatTreeModel())
{
}

QmitkXnatSimpleSearchView::~QmitkXnatSimpleSearchView()
{
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

void QmitkXnatSimpleSearchView::StartSearch()
{
  int type;

  switch(m_Controls.objectComboBox->currentIndex())
  {
  case QmitkXnatSimpleSearchView::PROJECT:
    type = QmitkXnatSimpleSearchView::PROJECT;
    break;
  case QmitkXnatSimpleSearchView::SUBJECT:
    type = QmitkXnatSimpleSearchView::SUBJECT;
    break;
  case QmitkXnatSimpleSearchView::EXPERIMENT:
    type = QmitkXnatSimpleSearchView::EXPERIMENT;
    break;
  default:
    type = QmitkXnatSimpleSearchView::EMPTY;
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

void QmitkXnatSimpleSearchView::SetSelectionProvider()
{
  GetSite()->SetSelectionProvider(m_SelectionProvider);
}
