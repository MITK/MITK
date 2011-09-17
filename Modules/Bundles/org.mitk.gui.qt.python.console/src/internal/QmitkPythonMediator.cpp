/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2010-09-07 13:46:44 +0200 (Di, 07 Sep 2010) $
Version:   $Revision: 25948 $ 

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "QmitkPythonMediator.h"
#include <dPython.h>

QmitkPythonMediator *QmitkPythonMediator::m_mediatorInstance = 0;

QmitkPythonMediator::QmitkPythonMediator()
{
}

QmitkPythonMediator::~QmitkPythonMediator()
{
}

void QmitkPythonMediator::Initialize()
{
  if(!Py_IsInitialized())
    Py_Initialize();
}

void QmitkPythonMediator::Finalize()
{
  if(m_clients.size() == 0)
    Py_Finalize();
}

QmitkPythonMediator *QmitkPythonMediator::getInstance()
{
  if(m_mediatorInstance == 0)
    m_mediatorInstance = new QmitkPythonMediator();
  return m_mediatorInstance;
}

void QmitkPythonMediator::runSimpleString(const char* str)
{
  if(!Py_IsInitialized()){
    Py_Initialize();
  }
  PyRun_SimpleString(str);
}

std::vector<QStringList> QmitkPythonMediator::getAttributeList()
{
  PyObject* dict = PyImport_GetModuleDict();
  PyObject* object = PyDict_GetItemString(dict, "__main__");    
  PyObject* dirMain = PyObject_Dir(object);
  PyObject* tempObject;
  std::vector<QStringList> variableStack;

  if(dirMain){
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

PyObject * QmitkPythonMediator::getPyObject(PyObject * object)
{
  PyObject* dict = PyImport_GetModuleDict();
  PyObject* main = PyDict_GetItemString(dict, "__main__");    
  return PyObject_GetAttr(main, object);
}

PyObject * QmitkPythonMediator::getPyObjectString(QString * objectName)
{
  PyObject* dict = PyImport_GetModuleDict();
  PyObject* main = PyDict_GetItemString(dict, "__main__");
  return PyObject_GetAttrString(main, objectName->toLocal8Bit().data());
}

void QmitkPythonMediator::setClient(QmitkPythonClient * client)
{
  m_clients.push_back(client);
}

void QmitkPythonMediator::unregisterClient(QmitkPythonClient * client)
{
  for(int i = 0; i < m_clients.size(); i++)
  {
    if(m_clients.at(i) == client)
      m_clients.erase(m_clients.begin()+i);
  }
}

void QmitkPythonMediator::update()
{
  if(Py_IsInitialized)
  {
    for(int i = 0; i < m_clients.size(); i++)
    {
      m_clients.at(i)->update();
    }
  }
}

void QmitkPythonMediator::SetCommandHistory(const QString& command)
{
  m_commandHistory.push_back(QStringList() << command);
}

std::vector<QStringList> QmitkPythonMediator::GetCommandHistory()
{
  return m_commandHistory;
}
