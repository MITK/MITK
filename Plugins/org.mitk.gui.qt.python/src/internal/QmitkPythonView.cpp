/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkPythonView.h"
#include <QmitkCtkPythonShell.h>
#include "mitkPluginActivator.h"
#include <QmitkPythonVariableStackTableView.h>
#include <QmitkPythonTextEditor.h>
#include <QmitkPythonSnippets.h>

#include <QHeaderView>
#include <QSplitter>
#include <QGridLayout>

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
    d->m_PythonVariableStackTableView = nullptr;
    d->m_PythonShell = nullptr;
}

QmitkPythonView::~QmitkPythonView()
{
    delete d;
}

void QmitkPythonView::CreateQtPartControl(QWidget* parent)
{
    d->m_PythonVariableStackTableView = new QmitkPythonVariableStackTableView;
    d->m_PythonVariableStackTableView->SetDataStorage(this->GetDataStorage());
    //d->m_PythonVariableStackTableView->horizontalHeader()->setResizeMode(QHeaderView::Interactive);

    QString snippetsFilePath = mitk::PluginActivator::m_XmlFilePath;
    MITK_DEBUG("QmitkPythonView") << "got snippetsFilePath " << snippetsFilePath.toStdString();

    d->m_PythonSnippets = new QmitkPythonSnippets(snippetsFilePath);

    MITK_DEBUG("QmitkPythonView") << "initializing varStackSnippetsTab";
    QTabWidget* varStackSnippetsTab = new QTabWidget;
    varStackSnippetsTab->addTab( d->m_PythonVariableStackTableView, "Variable Stack" );
    varStackSnippetsTab->addTab( d->m_PythonSnippets, "Snippets" );
    varStackSnippetsTab->setTabPosition( QTabWidget::South );

    MITK_DEBUG("QmitkPythonView") << "initializing m_PythonShell";
    d->m_PythonShell = new QmitkCtkPythonShell;

    MITK_DEBUG("QmitkPythonView") << "initializing m_TextEditor";
    d->m_TextEditor = new QmitkPythonTextEditor;

    MITK_DEBUG("QmitkPythonView") << "initializing tabWidgetConsoleEditor";
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

    MITK_DEBUG("QmitkPythonView") << "creating connections for m_PythonSnippets";
    connect( d->m_PythonSnippets, SIGNAL(PasteCommandRequested(QString)), d->m_PythonShell, SLOT(Paste(QString)) );
    connect( d->m_PythonSnippets, SIGNAL(PasteCommandRequested(QString)), d->m_TextEditor, SLOT(Paste(QString)) );
}

void QmitkPythonView::SetFocus()
{
    d->m_PythonShell->setFocus();
}
