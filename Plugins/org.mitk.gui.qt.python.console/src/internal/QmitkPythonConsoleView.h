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

#ifndef QmitkPythonConsoleView_h
#define QmitkPythonConsoleView_h

#include <berryISelectionListener.h>

#include <QmitkFunctionality.h>
#include "QmitkPythonEditor.h"

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
//#include "ctkPythonShell.h"
#include "QmitkCTKPythonShell.h"
#include "ctkAbstractPythonManager.h"
#include "QmitkPythonMediator.h"



/*!
  \brief QmitkPythonConsoleView 

  \warning  This application module is not yet documented. Use "svn blame/praise/annotate" and ask the author to provide basic documentation.

  \sa QmitkFunctionality
  \ingroup Functionalities
*/


class QmitkPythonConsoleView : public QmitkFunctionality, public QmitkPythonClient
{  
  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT
  
  public:  
    static const std::string VIEW_ID;

    QmitkPythonConsoleView();
    QmitkPythonConsoleView(const QmitkPythonConsoleView& other)
    {
       Q_UNUSED(other)
       throw std::runtime_error("Copy constructor not implemented");
    }
    virtual ~QmitkPythonConsoleView();

    virtual void CreateQtPartControl(QWidget *parent);

    virtual void StdMultiWidgetAvailable (QmitkStdMultiWidget &stdMultiWidget);
    virtual void StdMultiWidgetNotAvailable();
    virtual void NodeAdded(const mitk::DataNode* node);
    virtual void update();
    QmitkPythonMediator *getPythonMediator();

  protected slots:
    void OpenEditor();
    void SetCommandHistory(const QString&);

  protected:
    virtual void OnSelectionChanged( std::vector<mitk::DataNode*> nodes );
    QmitkStdMultiWidget* m_MultiWidget;

    berry::SmartPointer<QmitkPythonEditor> m_PythonEditor;
  private:
    FILE * m_scriptFile;
    QmitkCTKPythonShell *m_ctkPythonShell;
    ctkAbstractPythonManager *m_ctkPythonManager;
    QPushButton *m_ButtonOpenEditor;
    QmitkPythonMediator *m_PythonMediator;
};

#endif // _QMITKPYTHONCONSOLEVIEW_H_INCLUDED

