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

#include "QmitkPythonVariableStackTableModel.h"
#include <QMimeData>
#include <usModuleContext.h>
#include <mitkDataNode.h>
#include <usGetModuleContext.h>
#include <QStringList>
#include <QMessageBox>
#include "QmitkMimeTypes.h"

const QString QmitkPythonVariableStackTableModel::MITK_IMAGE_VAR_NAME = "mitkImage";
const QString QmitkPythonVariableStackTableModel::MITK_SURFACE_VAR_NAME = "mitkSurface";

QmitkPythonVariableStackTableModel::QmitkPythonVariableStackTableModel(QObject *parent)
    :QAbstractTableModel(parent)
{
    us::ModuleContext* context = us::GetModuleContext();
    m_PythonServiceRef = context->GetServiceReference<mitk::IPythonService>();
    m_PythonService = context->GetService<mitk::IPythonService>(m_PythonServiceRef);
    m_PythonService->AddPythonCommandObserver( this );
}

QmitkPythonVariableStackTableModel::~QmitkPythonVariableStackTableModel()
{
  us::ModuleContext* context = us::GetModuleContext();
  context->UngetService( m_PythonServiceRef );
  m_PythonService->RemovePythonCommandObserver( this );
}

bool QmitkPythonVariableStackTableModel::dropMimeData ( const QMimeData * data, Qt::DropAction action, int row, int column, const QModelIndex & parent )
{
    // Early exit, returning true, but not actually doing anything (ignoring data).
    if (action == Qt::IgnoreAction)
        return true;

    // Note, we are returning true if we handled it, and false otherwise
    bool returnValue = false;

    if(data->hasFormat(QmitkMimeTypes::DataNodePtrs))
    {
        MITK_DEBUG("QmitkPythonVariableStackTableModel") << "dropped MITK DataNode";
        returnValue = true;

        int i = 0;
        QList<mitk::DataNode*> dataNodeList = QmitkMimeTypes::ToDataNodePtrList(data);
        mitk::DataNode* node = NULL;
        foreach(node, dataNodeList)
        {
          mitk::Image* mitkImage = dynamic_cast<mitk::Image*>(node->GetData());
          MITK_DEBUG("QmitkPythonVariableStackTableModel") << "mitkImage is not null " << (mitkImage != 0? "true": "false");

          QRegExp rx("^\\d");
          QString varName(node->GetName().c_str());
          // regex replace every character that is not allowed in a python variable
          varName = varName.replace(QRegExp("[.\\+\\-*\\s\\/\\n\\t\\r]"),QString("_"));

          if( mitkImage )
          {
            if ( varName.isEmpty() )
              varName = MITK_IMAGE_VAR_NAME;
            if ( rx.indexIn(varName) == 0)
              varName.prepend("_").prepend(MITK_IMAGE_VAR_NAME);

            if( i > 0 )
              varName = QString("%1%2").arg(varName).arg(i);
            MITK_DEBUG("QmitkPythonVariableStackTableModel") << "varName" << varName.toStdString();

            bool exportAsCvImage = mitkImage->GetDimension() == 2 && m_PythonService->IsOpenCvPythonWrappingAvailable();

            if( exportAsCvImage )
            {
              int ret = QMessageBox::question(NULL, "Export option",
                "2D image detected. Export as OpenCV image to Python instead of an SimpleITK image?",
                                              QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
              exportAsCvImage = ret == QMessageBox::Yes;
              if(exportAsCvImage)
              {
                varName = MITK_IMAGE_VAR_NAME;
                m_PythonService->CopyToPythonAsCvImage( mitkImage, varName.toStdString() );
                ++i;
              }
            }
            if( !exportAsCvImage )
            {
              if( m_PythonService->IsSimpleItkPythonWrappingAvailable() )
              {
                m_PythonService->CopyToPythonAsSimpleItkImage( mitkImage, varName.toStdString() );
                ++i;
              }
              else
              {
                MITK_ERROR << "SimpleITK Python wrapping not available. Skipping export for image " << node->GetName();
              }
            }
          }
          else
          {
            mitk::Surface* surface = dynamic_cast<mitk::Surface*>(node->GetData());
            MITK_DEBUG("QmitkPythonVariableStackTableModel") << "found surface";

            if( surface )
            {
              if (varName.isEmpty() )
                varName =  MITK_SURFACE_VAR_NAME;
              if ( rx.indexIn(varName) == 0)
                varName.prepend("_").prepend(MITK_SURFACE_VAR_NAME);

              MITK_DEBUG("QmitkPythonVariableStackTableModel") << "varName" << varName;

              if( m_PythonService->IsVtkPythonWrappingAvailable() )
              {
                m_PythonService->CopyToPythonAsVtkPolyData( surface, varName.toStdString() );
              }
              else
              {
                MITK_ERROR << "VTK Python wrapping not available. Skipping export for surface " << node->GetName();
              }
            }
          }
        }
    }
    return returnValue;
}

QVariant QmitkPythonVariableStackTableModel::headerData(int section, Qt::Orientation orientation,
                                                        int role) const
{
    QVariant headerData;

    // show only horizontal header
    if ( role == Qt::DisplayRole )
    {
        if( orientation == Qt::Horizontal )
        {
            // first column: "Attribute"
            if(section == 0)
                headerData = "Attribute";
            else if(section == 1)
                headerData = "Type";
            else if(section == 2)
                headerData = "Value";
        }
    }

    return headerData;
}

Qt::ItemFlags QmitkPythonVariableStackTableModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags flags = QAbstractItemModel::flags(index);

    if(index.isValid())
        return Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | flags;
    else
        return Qt::ItemIsDropEnabled | flags;
}

int QmitkPythonVariableStackTableModel::rowCount(const QModelIndex &) const
{
    return m_VariableStack.size();
}

int QmitkPythonVariableStackTableModel::columnCount(const QModelIndex &) const
{
    return 3;
}

QVariant QmitkPythonVariableStackTableModel::data(const QModelIndex &index, int role) const
{
    if (index.isValid() && !m_VariableStack.empty())
    {
        if(role == Qt::DisplayRole)
        {
            mitk::PythonVariable item = m_VariableStack.at(index.row());
            if(index.column() == 0)
              return QString::fromStdString(item.m_Name);
            if(index.column() == 1)
                return QString::fromStdString(item.m_Type);
            if(index.column() == 2)
                return QString::fromStdString(item.m_Value);
        }
    }
    return QVariant();
}

QStringList QmitkPythonVariableStackTableModel::mimeTypes() const
{
    return QAbstractTableModel::mimeTypes();
    QStringList types;
    types << "application/x-mitk-datanodes";
    types << "application/x-qabstractitemmodeldatalist";
    return types;
}

Qt::DropActions QmitkPythonVariableStackTableModel::supportedDropActions() const
{
    return Qt::CopyAction | Qt::MoveAction;
}

void QmitkPythonVariableStackTableModel::CommandExecuted(const std::string& pythonCommand)
{
  MITK_DEBUG("QmitkPythonVariableStackTableModel") << "command was executed " << pythonCommand;
    m_VariableStack = m_PythonService->GetVariableStack();
    QAbstractTableModel::beginResetModel();
    QAbstractTableModel::endResetModel();
}

std::vector<mitk::PythonVariable> QmitkPythonVariableStackTableModel::GetVariableStack() const
{
    return m_VariableStack;
}
