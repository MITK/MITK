/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$ 
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef QmitkPythonCommandHistory_h
#define QmitkPythonCommandHistory_h

#include <berryISelectionListener.h>

#include <QmitkFunctionality.h>

#include <qstring.h>
#include <qfiledialog.h>
#include <stdio.h>

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QGridLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QPushButton>
#include <QtGui/QWidget>

#include "QmitkPythonCommandHistoryTreeWidget.h"
#include "QmitkPythonMediator.h"


class QmitkPythonCommandHistory : public QmitkFunctionality, public QmitkPythonClient
{  
  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT
  
  public:  
    static const std::string VIEW_ID;

    QmitkPythonCommandHistory();
    QmitkPythonCommandHistory(const QmitkPythonCommandHistory& other)
    {
       Q_UNUSED(other)
       throw std::runtime_error("Copy constructor not implemented");
    }
    virtual ~QmitkPythonCommandHistory();

    virtual void CreateQtPartControl(QWidget *parent);

    virtual void StdMultiWidgetAvailable (QmitkStdMultiWidget &stdMultiWidget);
    virtual void StdMultiWidgetNotAvailable();
    void SetCommandHistory(std::vector<QStringList>);
    virtual void update();

  public slots:
    void AddCommand(const QString&);

  protected slots:
  

  private:
    QmitkStdMultiWidget* m_MultiWidget;
    std::vector<QStringList> m_commandHistory;
    QmitkPythonCommandHistoryTreeWidget *m_treeWidget;
    QmitkPythonMediator *m_PythonMediator;
};

#endif // _QMITKPYTHONCONSOLEVIEW_H_INCLUDED

