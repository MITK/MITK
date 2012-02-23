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
  static QmitkPythonMediator m_mediatorInstance;
  return &m_mediatorInstance;
}

void QmitkPythonMediator::paste(const QString& pasteCommand)
{
  std::set<QmitkPythonPasteClient *>::iterator it
      = m_PasteClients.begin();

  while( it!= m_PasteClients.end())
  {
    (*it)->paste( pasteCommand );
    ++it;
  }
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

void  QmitkPythonMediator::registerPasteCommandClient(QmitkPythonPasteClient * client )
{
  m_PasteClients.insert( client );
}

void  QmitkPythonMediator::unregisterPasteCommandClient(QmitkPythonPasteClient * client)
{
  m_PasteClients.erase( client );
}

void QmitkPythonMediator::setClient(QmitkPythonClient * client)
{
  m_clients.insert(client);
}

void QmitkPythonMediator::unregisterClient(QmitkPythonClient * client)
{
  m_clients.erase( client );
}

void QmitkPythonMediator::update()
{
  if(Py_IsInitialized)
  {    
    std::set<QmitkPythonClient *>::iterator it
        = m_clients.begin();

    while( it!= m_clients.end())
    {
      (*it)->update();
      ++it;
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
