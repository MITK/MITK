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
#include "QmitkCtkPythonShell.h"
#include "mitkPluginActivator.h"
#include "QmitkPythonVariableStackTableModel.h"

const std::string QmitkPythonView::VIEW_ID = "org.mitk.views.python";

struct QmitkPythonViewData
{
    // widget
    QmitkPythonVariableStackTableModel* m_VariableStackTableModel;
    QmitkCtkPythonShell* m_PythonShell;
};

QmitkPythonView::QmitkPythonView()
    : d( new QmitkPythonViewData )
{
    d->m_PythonShell = 0;
}

QmitkPythonView::~QmitkPythonView()
{
    delete d;
}

void QmitkPythonView::CreateQtPartControl(QWidget* parent)
{
    d->m_VariableStackTableModel = new QmitkPythonVariableStackTableModel(parent);
    d->m_VariableStackTableModel->Update();

    QSplitter* splitter = new QSplitter;
    QTableView* variableStackView = new QTableView;
    variableStackView->setModel( d->m_VariableStackTableModel );

    d->m_PythonShell = new QmitkCtkPythonShell;
    d->m_PythonShell->SetPythonManager( mitk::PluginActivator::GetPythonManager() );

    splitter->addWidget(variableStackView);
    splitter->addWidget(d->m_PythonShell);

    QGridLayout* layout = new QGridLayout;
    layout->addWidget( splitter, 0, 0 );
    parent->setLayout(layout);

    connect( d->m_PythonShell, SIGNAL(newCommandExecuted()), d->m_VariableStackTableModel, SLOT(Update()) );
}

void QmitkPythonView::SetFocus()
{
    d->m_PythonShell->setFocus();
}
