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
#include <usGetModuleContext.h>
#include <mitkRenderingManager.h>
#include <mitkSurface.h>
#include <vtkPolyData.h>

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

    us::ModuleContext* context = us::GetModuleContext();
    us::ServiceReference<mitk::IPythonService> serviceRef = context->GetServiceReference<mitk::IPythonService>();
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

    if( type.startsWith("Image") )
    {
      mitkImage = m_PythonService->CopySimpleItkImageFromPython(varName.toStdString());
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
    mitk::DataNode::Pointer node = m_DataStorage->GetNamedNode(nodeName);

    // only create data node if it does not exist
    if ( node.IsNull() )
    {
      node = mitk::DataNode::New();
      node->SetName ( nodeName );
      m_DataStorage->Add(node);
    }

    if( mitkImage.IsNotNull() )
    {
      node->SetData( mitkImage );
    }
    else if( mitkSurface.IsNotNull() )
    {
      node->SetData( mitkSurface );
      // init renderwindow geometry
      mitk::RenderingManager::GetInstance()->InitializeViews(mitkSurface->GetGeometry());
    }

    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}
