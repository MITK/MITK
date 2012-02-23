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

#include "QmitkPythonVariableStack.h"

#include "mitkNodePredicateDataType.h"

#include "QmitkDataStorageComboBox.h"
#include "QmitkStdMultiWidget.h"
#include "QmitkPythonConsoleView.h"

#include "mitkDataStorageEditorInput.h"

// berry Includes
#include <berryPlatform.h>
#include <berryIWorkbenchPage.h>
#include <berryConstants.h>
#include <berryIViewPart.h>

#include <QMessageBox>
#include <QStringList>
#include <mitkImageCaster.h>
#include "QmitkPythonMediator.h"

const std::string QmitkPythonVariableStack::VIEW_ID = "org.mitk.views.pythonvariablestack";

QmitkPythonVariableStack::QmitkPythonVariableStack()
: m_MultiWidget(NULL)
, m_Controls(0)
{
}

QmitkPythonVariableStack::~QmitkPythonVariableStack()
{
  QmitkPythonMediator::getInstance()->unregisterClient(m_treeModel);
}

void QmitkPythonVariableStack::CreateQtPartControl(QWidget *parent)
{
  if( !m_Controls )
  {
    m_Controls = new Ui::QmitkPythonConsoleViewControls;
    m_Controls->setupUi( parent );
  }
  m_treeModel = new QmitkPythonVariableStackTreeWidget();
  m_Controls->tableView->setSelectionBehavior( QAbstractItemView::SelectRows );
  m_Controls->tableView->setAlternatingRowColors(true);
  m_Controls->tableView->setDragEnabled(true);
  m_Controls->tableView->setDropIndicatorShown(true);
  m_Controls->tableView->setAcceptDrops(true);
  m_Controls->tableView->setModel( m_treeModel);
  QmitkPythonMediator::getInstance()->setClient(m_treeModel);
  //QmitkPythonMediator::getInstance()->update();
  // add the multiwidget to the dings
  mitk::BaseRenderer* renderer  = mitk::BaseRenderer::GetInstance(
      this->GetActiveStdMultiWidget()->GetRenderWindow4()->GetVtkRenderWindow() );
  mitk::RendererAccess::Set3DRenderer( renderer->GetVtkRenderer() );
  m_treeModel->update();
}

QmitkPythonVariableStackTreeWidget* QmitkPythonVariableStack::getModel()
{
    return m_treeModel;
}

void QmitkPythonVariableStack::SetFocus ()
{

}

QmitkStdMultiWidget* QmitkPythonVariableStack::GetActiveStdMultiWidget()
{
  QmitkStdMultiWidget* activeStdMultiWidget = 0;
  berry::IEditorPart::Pointer editor =
    this->GetSite()->GetPage()->GetActiveEditor();

  if (editor.Cast<QmitkStdMultiWidgetEditor>().IsNotNull())
  {
    activeStdMultiWidget = editor.Cast<QmitkStdMultiWidgetEditor>()->GetStdMultiWidget();
  }
  else
  {
    mitk::DataStorageEditorInput::Pointer editorInput;
    editorInput = new mitk::DataStorageEditorInput();
    berry::IEditorPart::Pointer editor = this->GetSite()->GetPage()->OpenEditor(editorInput, QmitkStdMultiWidgetEditor::EDITOR_ID, false);
    activeStdMultiWidget = editor.Cast<QmitkStdMultiWidgetEditor>()->GetStdMultiWidget();
  }

  return activeStdMultiWidget;
}

int QmitkPythonVariableStack::GetSizeFlags(bool width)
{
  if(!width)
  {
    return berry::Constants::MIN | berry::Constants::MAX | berry::Constants::FILL;
  }
  else
  {
    return 0;
  }
}

int QmitkPythonVariableStack::ComputePreferredSize(bool width, int /*availableParallel*/, int /*availablePerpendicular*/, int preferredResult)
{
  if(width==false)
  {
    return 160;
  }
  else
  {
    return preferredResult;
  }
}
