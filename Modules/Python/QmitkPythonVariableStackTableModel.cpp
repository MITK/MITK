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
#include <mitkModuleContext.h>
#include <usServiceReference.h>
#include <mitkDataNode.h>
#include <mitkGetModuleContext.h>
#include <QStringList>

const QString QmitkPythonVariableStackTableModel::m_PythonImagesDictName("images");

QmitkPythonVariableStackTableModel::QmitkPythonVariableStackTableModel(QObject *parent)
    :QAbstractTableModel(parent)
{
    mitk::ModuleContext* context = mitk::GetModuleContext();
    mitk::ServiceReference serviceRef = context->GetServiceReference<mitk::IPythonService>();
    m_PythonService = context->GetService<mitk::IPythonService>(serviceRef);
}

QmitkPythonVariableStackTableModel::~QmitkPythonVariableStackTableModel()
{
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
        MITK_INFO << "dropped MITK DataNode";
        returnValue = true;

        QString arg = QString(data->data("application/x-mitk-datanodes").data());
        QStringList listOfDataNodeAddressPointers = arg.split(",");

        QStringList::iterator slIter;
        for (slIter = listOfDataNodeAddressPointers.begin();
             slIter != listOfDataNodeAddressPointers.end();
             slIter++)
        {
          long val = (*slIter).toLong();
          mitk::DataNode* node = static_cast<mitk::DataNode *>((void*)val);
          mitk::Image* mitkImage = dynamic_cast<mitk::Image*>(node->GetData());

          m_PythonService->Execute(CreateDictionaryCommandIfNecessary(), mitk::IPythonService::MULTI_LINE_COMMAND );

          QString varName = GetDictionaryVarNameForNodeName(node->GetName());
          MITK_DEBUG("varName") << "varName" << varName;
          m_PythonService->CopyToPythonAsItkImage( mitkImage, varName );
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
                headerData = "Value";
            else if(section == 2)
                headerData = "Type";
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
                return item.m_Name;
            if(index.column() == 1)
                return item.m_Value;
            if(index.column() == 2)
                return item.m_Type;
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

void QmitkPythonVariableStackTableModel::CommandExecuted(const QString &pythonCommand)
{
    m_VariableStack = m_PythonService->GetVariableStack();
    QAbstractTableModel::reset();
}

QList<mitk::PythonVariable> QmitkPythonVariableStackTableModel::GetVariableStack() const
{
    return m_VariableStack;
}


QString QmitkPythonVariableStackTableModel::CreateDictionaryCommandIfNecessary()
{
    QString command;
    command.append( QString("try:\n") );
    command.append( QString("  %1" ) .arg( m_PythonImagesDictName ) );
    command.append( QString("except NameError:\n") );
    command.append( QString("  %1 = {}\n") .arg( m_PythonImagesDictName ) );
    return command;
}

QString QmitkPythonVariableStackTableModel::GetDictionaryVarNameForNodeName(const std::string &nodeName)
{
    QString varName = QString("%1['%2']") .arg( m_PythonImagesDictName ) .arg( QString::fromStdString( nodeName ) );
    return varName;
}

std::string QmitkPythonVariableStackTableModel::GetNodeNameForDictionaryVarName(const QString &varName)
{
    QString qNodeName = varName;
    qNodeName = qNodeName.replace( m_PythonImagesDictName+"['", "" );
    qNodeName = qNodeName.replace("']", "");
    MITK_DEBUG("QmitkPythonVariableStackTableModel") << "qNodeName " << qNodeName.toStdString();
    return qNodeName.toStdString();
}
