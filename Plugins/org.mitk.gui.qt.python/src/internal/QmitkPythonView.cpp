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

#include "QmitkPythonView.h"
#include <QtGui>
#include <QmitkCtkPythonShell.h>
#include "mitkPluginActivator.h"
#include <QmitkPythonVariableStackTableView.h>
#include <QmitkPythonTextEditor.h>
#include <QmitkPythonSnippets.h>

const std::string QmitkPythonView::VIEW_ID = "org.mitk.views.python";

struct QmitkPythonViewData
{
    // widget
    QmitkPythonVariableStackTableView* m_PythonVariableStackTableView;
    QmitkPythonSnippets* m_PythonSnippets;

    QmitkCtkPythonShell* m_PythonShell;
    QmitkPythonTextEditor* m_TextEditor;
};

QmitkPythonView::QmitkPythonView()
    : d( new QmitkPythonViewData )
{
    d->m_PythonVariableStackTableView = 0;
    d->m_PythonShell = 0;
}

QmitkPythonView::~QmitkPythonView()
{
    delete d;
}

void QmitkPythonView::CreateQtPartControl(QWidget* parent)
{
    d->m_PythonVariableStackTableView = new QmitkPythonVariableStackTableView;
    d->m_PythonVariableStackTableView->SetDataStorage(this->GetDataStorage());
    d->m_PythonVariableStackTableView->horizontalHeader()->setResizeMode(QHeaderView::Stretch);

    d->m_PythonSnippets = new QmitkPythonSnippets;

    QTabWidget* varStackSnippetsTab = new QTabWidget;
    varStackSnippetsTab->addTab( d->m_PythonVariableStackTableView, "Variable Stack" );
    varStackSnippetsTab->addTab( d->m_PythonSnippets, "Snippets" );
    varStackSnippetsTab->setTabPosition( QTabWidget::South );

    d->m_PythonShell = new QmitkCtkPythonShell;

    d->m_TextEditor = new QmitkPythonTextEditor;

    QTabWidget* tabWidgetConsoleEditor = new QTabWidget;
    tabWidgetConsoleEditor->addTab( d->m_PythonShell, "Console" );
    tabWidgetConsoleEditor->addTab( d->m_TextEditor, "Text Editor" );
    tabWidgetConsoleEditor->setTabPosition( QTabWidget::South );

    QList<int> sizes;
    sizes << 1 << 3;
    QSplitter* splitter = new QSplitter;
    splitter->addWidget(varStackSnippetsTab);
    splitter->addWidget(tabWidgetConsoleEditor);
    splitter->setStretchFactor ( 0, 1 );
    splitter->setStretchFactor ( 1, 3 );

    QGridLayout* layout = new QGridLayout;
    layout->addWidget( splitter, 0, 0 );
    parent->setLayout(layout);

    connect( d->m_PythonSnippets, SIGNAL(PasteCommandRequested(QString)), d->m_PythonShell, SLOT(Paste(QString)) );
    connect( d->m_PythonSnippets, SIGNAL(PasteCommandRequested(QString)), d->m_TextEditor, SLOT(Paste(QString)) );
}

void QmitkPythonView::SetFocus()
{
    d->m_PythonShell->setFocus();
}
