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

#ifndef QmitkPythonMediator_h
#define QmitkPythonMediator_h

#include <iostream>
#include <vector>
#include <QStringList>
#include <dPython.h>
#include "mitkCommon.h"

class QmitkPythonClient
{
  public:
    virtual ~QmitkPythonClient(){};
    virtual void update() = 0;
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
    std::vector<QStringList> GetCommandHistory();
    void SetCommandHistory(const QString&);
    void Initialize();
    void Finalize();

  protected:
    QmitkPythonMediator();
    virtual ~QmitkPythonMediator();

  private:
    std::vector<QmitkPythonClient *> m_clients;
    std::vector<QStringList> m_commandHistory;
    std::vector<QStringList> m_variableStack;
    static QmitkPythonMediator *m_mediatorInstance;
};

#endif // _QMITKPYTHONCONSOLEVIEW_H_INCLUDED

