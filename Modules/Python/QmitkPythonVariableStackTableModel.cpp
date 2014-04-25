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

    if(data->hasFormat("application/x-mitk-datanodes"))
    {
        MITK_DEBUG("QmitkPythonVariableStackTableModel") << "dropped MITK DataNode";
        returnValue = true;

        QString arg = QString(data->data("application/x-mitk-datanodes").data());
        QStringList listOfDataNodeAddressPointers = arg.split(",");

        QStringList::iterator slIter;
        int i = 0;
        int j = 0;
        for (slIter = listOfDataNodeAddressPointers.begin();
             slIter != listOfDataNodeAddressPointers.end();
             slIter++)
        {
          size_t val = (*slIter).toULongLong();
          mitk::DataNode* node = reinterpret_cast<mitk::DataNode *>(val);
          mitk::Image* mitkImage = dynamic_cast<mitk::Image*>(node->GetData());
          MITK_DEBUG("QmitkPythonVariableStackTableModel") << "mitkImage is not null " << (mitkImage != 0? "true": "false");

          if( mitkImage )
          {
            QString varName = MITK_IMAGE_VAR_NAME;
            if( i > 0 )
              varName = QString("%1%2").arg(MITK_IMAGE_VAR_NAME).arg(i);
            MITK_DEBUG("QmitkPythonVariableStackTableModel") << "varName" << varName.toStdString();

            bool exportAsCvImage = mitkImage->GetDimension() == 2 && m_PythonService->IsOpenCvPythonWrappingAvailable();

            if( exportAsCvImage )
            {
              int ret = QMessageBox::question(NULL, "Export option",
                "2D image detected. Export as OpenCV image to Python instead of an ITK image?",
                                              QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

              exportAsCvImage = ret == QMessageBox::Yes;
              if(exportAsCvImage)
              {
                m_PythonService->CopyToPythonAsCvImage( mitkImage, MITK_IMAGE_VAR_NAME.toStdString() );
                ++i;
              }
            }
            if( !exportAsCvImage )
            {
              if( m_PythonService->IsItkPythonWrappingAvailable() )
              {
                m_PythonService->CopyToPythonAsItkImage( mitkImage, MITK_IMAGE_VAR_NAME.toStdString() );
                ++i;
              }
              else
              {
                MITK_ERROR << "ITK Python wrapping not available. Skipping export for image " << node->GetName();
              }
            }
          }
          else
          {
            mitk::Surface* surface = dynamic_cast<mitk::Surface*>(node->GetData());
            MITK_DEBUG("QmitkPythonVariableStackTableModel") << "found surface";

            if( surface )
            {
              QString varName = MITK_SURFACE_VAR_NAME;
              MITK_DEBUG("QmitkPythonVariableStackTableModel") << "varName" << varName.toStdString();

              if( j > 0 )
                varName = QString("%1%2").arg(MITK_SURFACE_VAR_NAME).arg(j);
              MITK_DEBUG("varName") << "varName" << varName.toStdString();

              if( m_PythonService->IsVtkPythonWrappingAvailable() )
              {
                m_PythonService->CopyToPythonAsVtkPolyData( surface, MITK_SURFACE_VAR_NAME.toStdString() );
                ++j;
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
    QAbstractTableModel::reset();
}

std::vector<mitk::PythonVariable> QmitkPythonVariableStackTableModel::GetVariableStack() const
{
    return m_VariableStack;
}
