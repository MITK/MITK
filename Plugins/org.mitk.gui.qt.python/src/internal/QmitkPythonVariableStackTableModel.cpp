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
#include <mitkImage.h>
#include <mitkImageAccessByItk.h>
#include <mitkIOUtil.h>
#include <mitkDataNode.h>
#include <QDir>
#include <QDateTime>
#include "mitkPluginActivator.h"
#include <ctkAbstractPythonManager.h>


QmitkPythonVariableStackTableModel::QmitkPythonVariableStackTableModel(QObject *parent)
    :QAbstractTableModel(parent)
{
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

          // save image
          QString tmpFolder = QDir::tempPath();
          QString nodeName = QString::fromStdString(node->GetName());
          QString fileName = tmpFolder + QDir::separator() + nodeName + ".nrrd";

          MITK_INFO << "Saving temporary file " << fileName.toStdString();
          if( !mitk::IOUtil::SaveImage(mitkImage, fileName.toStdString()) )
          {
              MITK_ERROR << "Temporary file could not be created.";
          }

          QString command;
          command.append("import itk\n");
          command.append( QString("dim_%1 = 3\n").arg( nodeName ) );
          command.append( QString("pixelType_%1 = itk.US\n").arg( nodeName ) );
          command.append( QString("imageType_%1 = itk.Image[pixelType_%2, dim_%3]\n").arg( nodeName ).arg( nodeName ).arg( nodeName ) );
          command.append( QString("readerType = itk.ImageFileReader[imageType_%1]\n").arg( nodeName ) );
          command.append( "reader = readerType.New()\n" );
          command.append( QString("reader.SetFileName( \"%1\" )\n").arg(fileName) );
          command.append("reader.Update()\n");
          command.append( QString("image_%1 = reader.GetOutput()\n").arg( nodeName ).arg( nodeName ) );
          MITK_INFO << "Issuing python command " << command.toStdString();
          mitk::PluginActivator::GetPythonManager()->executeString(command, ctkAbstractPythonManager::FileInput );
          this->Update();

          QFile file(fileName);
          MITK_INFO << "Removing file " << fileName.toStdString();
          file.remove();

          /*
          if( mitkImage )
          {
              MITK_INFO << "found MITK image";
              switch(mitkImage->GetDimension())
              {
                  case 2:
                    {
                      AccessFixedDimensionByItk( mitkImage, ItkImageProcessing, 2 ); break;
                    }
                  case 3:
                    {
                      AccessFixedDimensionByItk( mitkImage, ItkImageProcessing, 3 ); break;
                    }
                  case 4:
                    {
                      AccessFixedDimensionByItk( mitkImage, ItkImageProcessing, 4 ); break;
                    }
                  default: break;
              }
          }
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
              setVariableStack(this->getAttributeList());
          }
          */
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
            QStringList item = m_VariableStack.at(index.row());
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

void QmitkPythonVariableStackTableModel::clear()
{
    m_VariableStack.clear();
    QAbstractTableModel::reset();
}

void QmitkPythonVariableStackTableModel::setVariableStack(QList<QStringList> varStack)
{
    m_VariableStack = varStack;
    QAbstractTableModel::reset();
}

QList<QStringList> QmitkPythonVariableStackTableModel::getVariableStack()
{
    return m_VariableStack;
}

/*
QMimeData * QmitkPythonVariableStackTableModel::mimeData(const QModelIndexList & indexes) const
{
    QMimeData * ret = new QMimeData;

    QString command;
    QString tmpFolder = QDir::tempPath();
    for (int indexesCounter = 0; indexesCounter < indexes.size(); indexesCounter++)
    {
      QString name = m_VariableStack[indexes.at(indexesCounter).row()][0];
      QString fileName = tmpFolder + QDir::separator() + name + ".nrrd";

      MITK_INFO << "Saving temporary file with python itk code " << fileName.toStdString();
      command.append("import itk\n");
      command.append("writer = itk.ImageFileWriter[ image ].New()\n");
      command.append( QString( "reader.SetFileName( \"%1\" )\n").arg(fileName) );
      command.append( QString( "writer.SetInput( \"%1\" )\n").arg(fileName) );
      command.append("writer.Update()\n");

      MITK_INFO << "Loading temporary file " << fileName.toStdString() << " as MITK image";
      mitk::Image::Pointer mitkImage = mitk::IOUtil::LoadImage( fileName.toStdString() );

      if( mitkImage.IsNotNull() )
      {
      }
      else
      {
          MITK_ERROR << "Temporary image could not be written or was invalid.";
      }
    }

    return ret;

    /*
    QMimeData * ret = new QMimeData;

    QString dataNodeAddresses("");

    for (int indexesCounter = 0; indexesCounter < indexes.size(); indexesCounter++)
    {
      QString name = m_VariableStack[indexes.at(indexesCounter).row()][0];
      QString type = m_VariableStack[indexes.at(indexesCounter).row()][2];

      if(type != "DataNode_PointerPtr")
          return NULL;

      mitk::DataNode* node;
      itk::SmartPointer<mitk::DataNode> *arg;

      PyObject * obj = this->getPyObjectString(&name);
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
*/

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


void QmitkPythonVariableStackTableModel::Update()
{
//  std::cout << "QmitkPythonVariableStackTableModel::update()" << std::endl;
  this->setVariableStack(this->getAttributeList());
}

QList<QStringList> QmitkPythonVariableStackTableModel::getAttributeList()
{
  PyObject* dict = PyImport_GetModuleDict();
  PyObject* object = PyDict_GetItemString(dict, "__main__");
  PyObject* dirMain = PyObject_Dir(object);
  PyObject* tempObject;
  QList<QStringList> variableStack;

  if(dirMain)
  {
    QString attr, attrValue, attrType;
    variableStack.clear();
    for(int i = 0; i<PyList_Size(dirMain); i++)
    {
      tempObject = PyList_GetItem(dirMain, i);
      attr = PyString_AsString(tempObject);
      tempObject = PyObject_GetAttrString(object, attr.toLocal8Bit().data());
      attrType = tempObject->ob_type->tp_name;
      if(PyUnicode_Check(tempObject) || PyString_Check(tempObject))
        attrValue = PyString_AsString(tempObject);
      else
        attrValue = "";
      variableStack.push_back(QStringList() << attr << attrValue << attrType);
    }
  }
  return variableStack;
}

PyObject * QmitkPythonVariableStackTableModel::getPyObject(PyObject * object)
{
  PyObject* dict = PyImport_GetModuleDict();
  PyObject* main = PyDict_GetItemString(dict, "__main__");
  return PyObject_GetAttr(main, object);
}

PyObject * QmitkPythonVariableStackTableModel::getPyObjectString(const QString &objectName)
{
  PyObject* dict = PyImport_GetModuleDict();
  PyObject* main = PyDict_GetItemString(dict, "__main__");
  return PyObject_GetAttrString(main, objectName.toLocal8Bit().data());
}
