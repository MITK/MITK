/*======================================================================
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "QmitkPythonVariableStackTreeWidget.h"

#include <QMouseEvent>
#include <QUrl>
#include <QDataStream>
#include <QByteArray>
#include <QTextStream>
#include <QString>
#include <QTreeWidgetItem>

#include <wrap_mitkDataNodePython.cxx>

#include "QmitkPythonMediator.h"
#include "QmitkCustomVariants.h"
#include "QmitkNodeDescriptor.h"
#include "QmitkNodeDescriptorManager.h"
#include "mitkDataStorage.h"
#include "mitkDataNode.h"

QmitkPythonVariableStackTreeWidget::QmitkPythonVariableStackTreeWidget(QWidget* parent)
    :QAbstractTableModel(parent)
{
}

QmitkPythonVariableStackTreeWidget::~QmitkPythonVariableStackTreeWidget()
{
}

bool QmitkPythonVariableStackTreeWidget::dropMimeData ( const QMimeData * data, Qt::DropAction action, int row, int column, const QModelIndex & parent )
{
    // Early exit, returning true, but not actually doing anything (ignoring data).
    if (action == Qt::IgnoreAction)
        return true;

    // Note, we are returning true if we handled it, and false otherwise
    bool returnValue = false;

    if(data->hasFormat("application/x-mitk-datanodes"))
    {        
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

          itk::SmartPointer<mitk::DataNode > * resultptr;
          resultptr = new itk::SmartPointer<mitk::DataNode >((itk::SmartPointer<mitk::DataNode > &)node);

          if(resultptr)
          {
              int i = 0;
              while(swig_types_initial[i] != 0)
              {
                if(swig_types_initial[i] == _swigt__p_itk__SmartPointerTmitk__DataNode_t)
                  SWIGTYPE_p_itk__SmartPointerTmitk__DataNode_t = SWIG_TypeRegister(swig_types_initial[i]);
                i++;
              }

              PyObject * resultObj = SWIG_NewPointerObj((void*)(resultptr), SWIGTYPE_p_itk__SmartPointerTmitk__DataNode_t, 1);
              PyObject* dict = PyImport_GetModuleDict();
              PyObject* object = PyDict_GetItemString(dict, "__main__");

              if(object){
                  Py_INCREF(object);
                  PyModule_AddObject(object, node->GetName().c_str(), resultObj);
              }
              setVariableStack(QmitkPythonMediator::getAttributeList());
          }
        }

    }
    return returnValue;
}

QVariant QmitkPythonVariableStackTreeWidget::headerData(int section, Qt::Orientation orientation,
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

Qt::ItemFlags QmitkPythonVariableStackTreeWidget::flags(const QModelIndex &index) const
{
    Qt::ItemFlags flags = QAbstractItemModel::flags(index);

    if(index.isValid())
        return Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | flags;
    else
        return Qt::ItemIsDropEnabled | flags;
}

int QmitkPythonVariableStackTreeWidget::rowCount(const QModelIndex &) const
{
    return m_variableStack.size();
}

int QmitkPythonVariableStackTreeWidget::columnCount(const QModelIndex &) const
{
    return 3;
}

QVariant QmitkPythonVariableStackTreeWidget::data(const QModelIndex &index, int role) const
{
    if (index.isValid() && !m_variableStack.empty())
    {
        if(role == Qt::DisplayRole)
        {
            QStringList item = m_variableStack.at(index.row());
            if(index.column() == 0)
                return item[0];
            if(index.column() == 1)
                return item[1];
            if(index.column() == 2)
                return item[2];
        }
    }
    return QVariant();
}

void QmitkPythonVariableStackTreeWidget::clear()
{
    m_variableStack.clear();
    QAbstractTableModel::reset();
}

void QmitkPythonVariableStackTreeWidget::setVariableStack(std::vector<QStringList> varStack)
{
    m_variableStack = varStack;
    QAbstractTableModel::reset();
}

std::vector<QStringList> QmitkPythonVariableStackTreeWidget::getVariableStack()
{
    return m_variableStack;
}

QMimeData * QmitkPythonVariableStackTreeWidget::mimeData(const QModelIndexList & indexes) const
{

    QMimeData * ret = new QMimeData;

    QString dataNodeAddresses("");

    for (int indexesCounter = 0; indexesCounter < indexes.size(); indexesCounter++)
    {
      QString name = m_variableStack[indexes.at(indexesCounter).row()][0];
      QString type = m_variableStack[indexes.at(indexesCounter).row()][2];

      if(type != "DataNode_PointerPtr")
          return NULL;

      mitk::DataNode* node;
      itk::SmartPointer<mitk::DataNode> *arg;

      PyObject * obj = QmitkPythonMediator::getPyObjectString(&name);
      int i = 0;
      while(swig_types_initial[i] != 0)
      {
        if(swig_types_initial[i] == _swigt__p_itk__SmartPointerTmitk__DataNode_t)
          SWIGTYPE_p_itk__SmartPointerTmitk__DataNode_t = SWIG_TypeRegister(swig_types_initial[i]);
        i++;
      }
      if ((SWIG_ConvertPtr(obj,(void **)(&arg),SWIGTYPE_p_itk__SmartPointerTmitk__DataNode_t,
                           SWIG_POINTER_EXCEPTION | 0)) == -1) return NULL;

      if(arg == NULL){
          return NULL;
      }
      try {
          node = (mitk::DataNode *)((itk::SmartPointer<mitk::DataNode > const *)arg)->GetPointer();
      }
      catch(std::exception &_e) {
          {
              std::cout << "Not a DataNode" << std::endl;
          }
      }

      long a = reinterpret_cast<long>(node);

      if (dataNodeAddresses.size() != 0)
      {
        QTextStream(&dataNodeAddresses) << ",";
      }
      QTextStream(&dataNodeAddresses) << a;

      ret->setData("application/x-mitk-datanodes", QByteArray(dataNodeAddresses.toAscii()));
    }

    return ret;
}

QStringList QmitkPythonVariableStackTreeWidget::mimeTypes() const
{
    QStringList types;
    types << "application/x-mitk-datanodes";
    types << "application/x-qabstractitemmodeldatalist";
    return types;
}

Qt::DropActions QmitkPythonVariableStackTreeWidget::supportedDropActions() const
{
    return Qt::CopyAction | Qt::MoveAction;
}

Qt::DropActions QmitkPythonVariableStackTreeWidget::supportedDragActions() const
{
    return Qt::CopyAction | Qt::MoveAction;
}

void QmitkPythonVariableStackTreeWidget::update()
{
//  std::cout << "QmitkPythonVariableStackTreeWidget::update()" << std::endl;
  setVariableStack(QmitkPythonMediator::getAttributeList());
}
