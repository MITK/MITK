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

// Blueberry
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>
#include <berryIWorkbenchPage.h>

// Qmitk
#include "QmitkPythonCommandHistory.h"
#include "QmitkPythonConsoleView.h"
#include "QmitkStdMultiWidget.h"

// Qt
#include <QMessageBox>



#include "mitkCoreObjectFactory.h"
#include <mitkIDataStorageReference.h>
#include <mitkIDataStorageService.h>
#include <mitkDataStorageEditorInput.h>
#include <dPython.h>
#include <QMetaType>

#include "berryFileEditorInput.h"
#include <berryIViewPart.h>

const std::string QmitkPythonCommandHistory::VIEW_ID = "org.mitk.views.pythoncommandhistory";

QmitkPythonCommandHistory::QmitkPythonCommandHistory()
: QmitkFunctionality()
//, m_Controls( 0 )
, m_MultiWidget( NULL )
{
}

QmitkPythonCommandHistory::~QmitkPythonCommandHistory()
{
  QmitkPythonMediator::getInstance()->unregisterClient(this);
}


void QmitkPythonCommandHistory::CreateQtPartControl( QWidget *parent )
{
  QGridLayout *gridLayout;


  if (parent->objectName().isEmpty())
    parent->setObjectName(QString::fromUtf8("parent"));
  parent->resize(790, 773);
  parent->setMinimumSize(QSize(0, 0));
  gridLayout = new QGridLayout(parent);
  gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
  m_treeWidget = new QmitkPythonCommandHistoryTreeWidget(parent);
  QTreeWidgetItem *treeWidgetItem = new QTreeWidgetItem();
  treeWidgetItem->setText(0, QString::fromUtf8("Command"));
  treeWidgetItem->setFlags(Qt::ItemIsEditable);
  m_treeWidget->setHeaderItem(treeWidgetItem);
  m_treeWidget->setObjectName(QString::fromUtf8("treeWidget"));
  m_treeWidget->setColumnCount(1);
  m_treeWidget->setDragEnabled(true);
  m_treeWidget->setDragDropMode(QAbstractItemView::DragOnly);
  m_treeWidget->setAcceptDrops(false);
  m_treeWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);

  gridLayout->addWidget(m_treeWidget, 0, 0, 1, 1);
  parent->setWindowTitle(QApplication::translate("QmitkPythonConsoleViewControls", "QmitkTemplate", 0, QApplication::UnicodeUTF8));

  QMetaObject::connectSlotsByName(parent);
  QmitkPythonMediator::getInstance()->setClient(this);
}


void QmitkPythonCommandHistory::StdMultiWidgetAvailable (QmitkStdMultiWidget &stdMultiWidget)
{
  m_MultiWidget = &stdMultiWidget;
}


void QmitkPythonCommandHistory::StdMultiWidgetNotAvailable()
{
  m_MultiWidget = NULL;
}

void QmitkPythonCommandHistory::AddCommand(const QString& command)
{
  m_commandHistory.push_back(QStringList() << command);
  m_treeWidget->clear();
  for(int i = 0; i < m_commandHistory.size(); i++)
  {
    if( m_commandHistory[i].value(0).compare("") != 0 )
      m_treeWidget->addTopLevelItem(new QTreeWidgetItem(m_commandHistory[i]));
  }
}

void QmitkPythonCommandHistory::SetCommandHistory(std::vector<QStringList> history)
{
  m_commandHistory = history;
  m_treeWidget->clear();
  for(int i = 0; i < m_commandHistory.size(); i++)
  {
    if( m_commandHistory[i].value(0).compare("") != 0 )
      m_treeWidget->addTopLevelItem(new QTreeWidgetItem(m_commandHistory[i]));
  }
}

void QmitkPythonCommandHistory::update()
{
  SetCommandHistory(QmitkPythonMediator::getInstance()->GetCommandHistory());
}
