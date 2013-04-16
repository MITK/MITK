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
#include <mitkSurface.h>

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

    int row = index.row();
    std::vector<mitk::PythonVariable> variableStack = m_TableModel->GetVariableStack();
    {
      MITK_DEBUG("QmitkPythonVariableStackTableView") << "row " << row;
      MITK_DEBUG("QmitkPythonVariableStackTableView") << "variableStack.size(): " << variableStack.size();
    }

    QString varName = QString::fromStdString( variableStack.at(row).m_Name );
    QString type = QString::fromStdString( variableStack.at(row).m_Type );
    QString value = QString::fromStdString( variableStack.at(row).m_Value );

    {
      MITK_DEBUG("QmitkPythonVariableStackTableView") << "varName: " << varName.toStdString();
      MITK_DEBUG("QmitkPythonVariableStackTableView") << "type: " << type.toStdString();
    }

    mitk::Image::Pointer mitkImage;
    mitk::Surface::Pointer mitkSurface;

    if( type.startsWith("itkImage") )
    {
      mitkImage = m_PythonService->CopyItkImageFromPython(varName.toStdString());
    }
    else if( type.startsWith("numpy.ndarray") )
    {
      mitkImage = m_PythonService->CopyCvImageFromPython(varName.toStdString());
    }
    else if( value.startsWith("(vtkPolyData)") )
    {
      mitkSurface = m_PythonService->CopyVtkPolyDataFromPython(varName.toStdString());
    }

    std::string nodeName = varName.toStdString();
    mitk::DataNode::Pointer node = mitk::DataNode::New();
    node->SetName ( nodeName );

    if( mitkImage.IsNotNull() )
    {
      node->SetData( mitkImage );
    }
    else if( mitkSurface.IsNotNull() )
    {
      node->SetData( mitkSurface );
    }

    m_DataStorage->Add(node);
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}
