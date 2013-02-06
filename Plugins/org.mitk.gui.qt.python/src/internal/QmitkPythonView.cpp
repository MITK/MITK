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
#include "QmitkPythonVariableStackTableView.h"

const std::string QmitkPythonView::VIEW_ID = "org.mitk.views.python";

struct QmitkPythonViewData
{
    // widget
    QmitkPythonVariableStackTableView* m_PythonVariableStackTableView;
    QmitkCtkPythonShell* m_PythonShell;
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

    d->m_PythonShell = new QmitkCtkPythonShell;

    QList<int> sizes;
    sizes << 1 << 3;
    QSplitter* splitter = new QSplitter;
    splitter->addWidget(d->m_PythonVariableStackTableView);
    splitter->addWidget(d->m_PythonShell);
    splitter->setStretchFactor ( 0, 1 );
    splitter->setStretchFactor ( 1, 3 );

    QGridLayout* layout = new QGridLayout;
    layout->addWidget( splitter, 0, 0 );
    parent->setLayout(layout);
}

void QmitkPythonView::SetFocus()
{
    d->m_PythonShell->setFocus();
}
