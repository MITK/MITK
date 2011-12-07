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

// Blueberry
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>
#include <berryIWorkbenchPage.h>

// Qmitk
#include "QmitkPythonConsoleView.h"
#include "QmitkStdMultiWidget.h"

// Qt
#include <QMessageBox>
#include <QStringList>

#include "mitkCoreObjectFactory.h"
#include <mitkIDataStorageReference.h>
#include <mitkIDataStorageService.h>
#include <mitkRenderingManager.h>
#include <mitkDataStorageEditorInput.h>
#include <dPython.h>
#include <QMetaType>
//#include <wrap_mitkDataStoragePython.cxx>

#include "berryFileEditorInput.h"
#include <berryIViewPart.h>

#include "QmitkPythonVariableStack.h"
#include "QmitkPythonCommandHistory.h"
#include "mitkPythonPath.h"


const std::string QmitkPythonConsoleView::VIEW_ID = "org.mitk.views.pythonconsole"; 

QmitkPythonConsoleView::QmitkPythonConsoleView()
: QmitkFunctionality()
//, m_Controls( 0 )
, m_MultiWidget( NULL )
,m_ctkPythonManager( NULL )
,m_ctkPythonShell( NULL )
{  
  // TODO: take it to the activator
//  std::cout << "running PYTHON PATH COMMANDS" << std::endl;

  QmitkPythonMediator::getInstance()->runSimpleString("import sys");

  std::string cmd ( "sys.path.append('" MITK_PYTHONPATH_MITK_LIBRARY_DIRS "');" );
  QmitkPythonMediator::getInstance()->runSimpleString(cmd.c_str());

  cmd = "sys.path.append('" MITK_PYTHONPATH_ITK_LIBRARY_DIRS "');";
  QmitkPythonMediator::getInstance()->runSimpleString(cmd.c_str());

  cmd = "sys.path.append('" MITK_PYTHONPATH_WRAP_ITK_DIR "');";
  QmitkPythonMediator::getInstance()->runSimpleString(cmd.c_str());

  cmd = "sys.path.append('" MITK_PYTHONPATH_VTK_LIBRARY_DIRS "');";
  QmitkPythonMediator::getInstance()->runSimpleString(cmd.c_str());

  cmd = "sys.path.append('" MITK_PYTHONPATH_VTK_PYTHON_WRAPPING_DIR "');";
  QmitkPythonMediator::getInstance()->runSimpleString(cmd.c_str());

  if( strcmp("", MITK_PYTHONPATH_OPEN_CV_LIBRARY_DIRS) != 0 )
  {
//    std::cout << "setting opencv PYTHON PATH" << std::endl;
    cmd = "sys.path.append('" MITK_PYTHONPATH_OPEN_CV_LIBRARY_DIRS "');";
    QmitkPythonMediator::getInstance()->runSimpleString(cmd.c_str());
  }
}

QmitkPythonConsoleView::~QmitkPythonConsoleView()
{
  QmitkPythonMediator::getInstance()->unregisterClient(this);
}


void QmitkPythonConsoleView::CreateQtPartControl( QWidget *parent )
{
    QGridLayout *gridLayout;
    if (parent->objectName().isEmpty())
            parent->setObjectName(QString::fromUtf8("parent"));
    parent->resize(790, 774);
    parent->setMinimumSize(QSize(0, 0));
    gridLayout = new QGridLayout(parent);
    gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
    m_ctkPythonManager = new ctkAbstractPythonManager(parent);
    m_ctkPythonShell = new QmitkCTKPythonShell(m_ctkPythonManager, parent);
    m_ctkPythonShell->setObjectName(QString::fromUtf8("m_ctkPythonShell"));
    m_ctkPythonShell->setMinimumSize(QSize(120, 100));
    m_ctkPythonShell->setMaximumSize(QSize(2000, 2000));
    gridLayout->addWidget(m_ctkPythonShell, 0, 0, 1, 1);
    m_ButtonOpenEditor = new QPushButton(parent);
    m_ButtonOpenEditor->setObjectName(QString::fromUtf8("m_ButtonOpenEditor"));
    gridLayout->addWidget(m_ButtonOpenEditor, 1, 0, 1, 1);
    parent->setWindowTitle(QApplication::translate("parent", "QmitkTemplate", 0, QApplication::UnicodeUTF8));
    m_ButtonOpenEditor->setText(QApplication::translate("parent", "Open Editor", 0, QApplication::UnicodeUTF8));
    QMetaObject::connectSlotsByName(parent);    
    mitk::DataStorage::Pointer dataStorage = this->GetDefaultDataStorage();

    m_ctkPythonManager->executeString("import mitk");
    QmitkPythonMediator::getInstance()->setClient(this);
    QmitkPythonMediator::getInstance()->update();

    connect( m_ctkPythonShell, SIGNAL(executeCommandSignal(const QString&)),
             this, SLOT(SetCommandHistory(const QString&)));
    connect( m_ButtonOpenEditor, SIGNAL(clicked()), this, SLOT(OpenEditor()) );
}


void QmitkPythonConsoleView::StdMultiWidgetAvailable (QmitkStdMultiWidget &stdMultiWidget)
{
  m_MultiWidget = &stdMultiWidget;
}


void QmitkPythonConsoleView::StdMultiWidgetNotAvailable()
{
  m_MultiWidget = NULL;
}


void QmitkPythonConsoleView::OnSelectionChanged( std::vector<mitk::DataNode*> nodes )
{ 
  for( std::vector<mitk::DataNode*>::iterator it = nodes.begin();
       it != nodes.end();
       ++it )
  {
    mitk::DataNode::Pointer node = *it;
  
    if( node.IsNotNull() && dynamic_cast<mitk::Image*>(node->GetData()) )
    {
      return;
    }
  }
}

void QmitkPythonConsoleView::NodeAdded(const mitk::DataNode* node)
{
}

void QmitkPythonConsoleView::OpenEditor()
{
    berry::FileEditorInput::Pointer editorInput;
    editorInput = new berry::FileEditorInput("");
    m_PythonEditor = this->GetSite()->GetPage()->OpenEditor(editorInput, QmitkPythonEditor::EDITOR_ID, true, berry::IWorkbenchPage::MATCH_NONE).Cast<QmitkPythonEditor>();
    
}

void QmitkPythonConsoleView::SetCommandHistory(const QString& command)
{
  QmitkPythonMediator::getInstance()->SetCommandHistory(command);
//  QmitkPythonMediator::getInstance()->update();
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkPythonConsoleView::update()
{
  //QmitkPythonMediator::runSimpleString("\r\n");
}

QmitkPythonMediator *QmitkPythonConsoleView::getPythonMediator()
{
  return m_PythonMediator;
}
