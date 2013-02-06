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
#include <mitkIOUtil.h>
#include <ctkAbstractPythonManager.h>

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

void QmitkPythonView::OnVariableStackDoubleClicked(const QModelIndex &index)
{
    QString command;
    QString tmpFolder = QDir::tempPath();

    QString name = d->m_VariableStackTableModel->getVariableStack().at(index.row())[0];
    QString fileName = tmpFolder + QDir::separator() + name + ".nrrd";

    MITK_INFO << "Saving temporary file with python itk code " << fileName.toStdString();
    command.append("import itk\n");
    command.append( QString( "writer = itk.ImageFileWriter[ %1 ].New()\n").arg( name ) );
    command.append( QString( "writer.SetFileName( \"%1\" )\n").arg(fileName) );
    command.append( QString( "writer.SetInput( %1 )\n").arg(name) );
    command.append("writer.Update()\n");
    mitk::PluginActivator::GetPythonManager()->executeString(command, ctkAbstractPythonManager::FileInput );
    d->m_VariableStackTableModel->Update();

    mitk::Image::Pointer mitkImage;
    try
    {
        MITK_INFO << "Loading temporary file " << fileName.toStdString() << " as MITK image";
        mitkImage = mitk::IOUtil::LoadImage( fileName.toStdString() );
    }
    catch(std::exception& e)
    {
        MITK_ERROR << e.what();
    }

    QFile file(fileName);
    if( file.exists() )
    {
        MITK_INFO << "Removing temporary file " << fileName.toStdString();
        file.remove();
    }

    if( mitkImage.IsNotNull() )
    {
        QString nodeName = name.replace("image_", "");
        mitk::DataNode::Pointer node = this->GetDataStorage()->GetNamedNode( nodeName.toStdString().c_str() );
        if( node.IsNull() )
        {
            node = mitk::DataNode::New();
            this->GetDataStorage()->Add(node);
        }

        node->SetData( mitkImage );

        mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    }
}

void QmitkPythonView::CreateQtPartControl(QWidget* parent)
{
    d->m_VariableStackTableModel = new QmitkPythonVariableStackTableModel(parent);
    d->m_VariableStackTableModel->Update();

    QTableView* variableStackView = new QTableView;
    variableStackView->setSelectionBehavior( QAbstractItemView::SelectRows );
    variableStackView->setAlternatingRowColors(true);
    //variableStackView->setDragEnabled(true);
    variableStackView->setDropIndicatorShown(true);
    variableStackView->setAcceptDrops(true);
    variableStackView->setModel( d->m_VariableStackTableModel );

    d->m_PythonShell = new QmitkCtkPythonShell;
    d->m_PythonShell->SetPythonManager( mitk::PluginActivator::GetPythonManager() );

    QList<int> sizes;
    sizes << 1 << 3;
    QSplitter* splitter = new QSplitter;
    splitter->addWidget(variableStackView);
    splitter->addWidget(d->m_PythonShell);
    splitter->setStretchFactor ( 0, 1 );
    splitter->setStretchFactor ( 1, 3 );

    QGridLayout* layout = new QGridLayout;
    layout->addWidget( splitter, 0, 0 );
    parent->setLayout(layout);

    connect( d->m_PythonShell, SIGNAL(newCommandExecuted()), d->m_VariableStackTableModel, SLOT(Update()) );
    connect( variableStackView, SIGNAL(doubleClicked ( const QModelIndex& )), this, SLOT( OnVariableStackDoubleClicked(const QModelIndex&) ) );
}

void QmitkPythonView::SetFocus()
{
    d->m_PythonShell->setFocus();
}
