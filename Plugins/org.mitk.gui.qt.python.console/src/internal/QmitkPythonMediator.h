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

#ifndef QmitkPythonMediator_h
#define QmitkPythonMediator_h

#include <iostream>
#include <vector>
#include <set>
#include <QStringList>
#include <dPython.h>
#include "mitkCommon.h"

class QmitkPythonClient
{
  public:
    virtual ~QmitkPythonClient(){};
    virtual void update() = 0;
};

class QmitkPythonPasteClient
{
  public:
    virtual ~QmitkPythonPasteClient(){};
    virtual void paste(const QString&) = 0;
};

class QmitkPythonMediator
{
  public:
    void update();

    static void runSimpleString(const char*);
    static std::vector<QStringList> getAttributeList();
    static PyObject * getPyObject(PyObject * object);
    static PyObject * getPyObjectString(QString * objectName);
    static QmitkPythonMediator *getInstance();

    void setClient(QmitkPythonClient *);
    void unregisterClient(QmitkPythonClient *);
    void registerPasteCommandClient(QmitkPythonPasteClient *);
    void unregisterPasteCommandClient(QmitkPythonPasteClient *);
    void paste(const QString& pasteCommand);
    std::vector<QStringList> GetCommandHistory();
    void SetCommandHistory(const QString&);
    void Initialize();
    void Finalize();

  protected:
    QmitkPythonMediator();
    virtual ~QmitkPythonMediator();

  private:
    std::set<QmitkPythonClient *> m_clients;
    std::set<QmitkPythonPasteClient *> m_PasteClients;
    std::vector<QStringList> m_commandHistory;
    std::vector<QStringList> m_variableStack;
};

#endif // _QMITKPYTHONCONSOLEVIEW_H_INCLUDED

