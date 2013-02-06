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

#include "QmitkPythonVariableStackTableView.h"
#include <usModuleContext.h>
#include <usServiceReference.h>
#include <mitkGetModuleContext.h>
#include <mitkRenderingManager.h>

QmitkPythonVariableStackTableView::QmitkPythonVariableStackTableView(QWidget *parent)
    :QTableView(parent)
{
    m_TableModel = new QmitkPythonVariableStackTableModel(parent);
    m_TableModel->CommandExecuted("");

    this->setSelectionBehavior( QAbstractItemView::SelectRows );
    this->setAlternatingRowColors(true);
    this->setDropIndicatorShown(true);
    this->setAcceptDrops(true);
    this->setModel( m_TableModel );

    mitk::ModuleContext* context = mitk::GetModuleContext();
    mitk::ServiceReference serviceRef = context->GetServiceReference<mitk::IPythonService>();
    m_PythonService = context->GetService<mitk::IPythonService>(serviceRef);

    connect( this, SIGNAL(doubleClicked ( const QModelIndex& )), this, SLOT( OnVariableStackDoubleClicked(const QModelIndex&) ) );
}

QmitkPythonVariableStackTableView::~QmitkPythonVariableStackTableView()
{
}

void QmitkPythonVariableStackTableView::SetDataStorage(mitk::DataStorage *_DataStorage)
{
    m_DataStorage = _DataStorage;
}

void QmitkPythonVariableStackTableView::OnVariableStackDoubleClicked(const QModelIndex &index)
{
    if( m_DataStorage.IsNull() || m_PythonService == 0 )
    {
        MITK_ERROR << "QmitkPythonVariableStackTableView not configured correctly. Quit";
        return;
    }

    QList<mitk::PythonVariable> variableStack = m_TableModel->GetVariableStack();
    QString varName = variableStack.at(index.row()).m_Name;

    MITK_DEBUG("QmitkPythonVariableStackTableView") << varName;
    mitk::Image::Pointer mitkImage = m_PythonService->CopyItkImageFromPython(varName);

    if( mitkImage.IsNotNull() )
    {
        std::string nodeName = QmitkPythonVariableStackTableModel::GetNodeNameForDictionaryVarName(varName);

        mitk::DataNode::Pointer node = m_DataStorage->GetNamedNode( nodeName.c_str() );
        if( node.IsNull() )
        {
            node = mitk::DataNode::New();
            m_DataStorage->Add(node);
        }

        node->SetData( mitkImage );

        mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    }
}
