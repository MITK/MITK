/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date$
Version:   $Revision: 17495 $ 
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "QmitkImageNavigatorView.h"

#include "mitkNodePredicateDataType.h"

#include "QmitkDataStorageComboBox.h"
#include "QmitkStdMultiWidget.h"

#include "mitkDataStorageEditorInput.h"

// berry Includes
#include <berryPlatform.h>
#include <berryIWorkbenchPage.h>
#include <berryConstants.h>

#include <QMessageBox>



const std::string QmitkImageNavigatorView::VIEW_ID = "org.mitk.views.imagenavigator";

QmitkImageNavigatorView::QmitkImageNavigatorView()
: m_MultiWidget(NULL)
{
}

QmitkImageNavigatorView::~QmitkImageNavigatorView()
{
  //delete m_TransversalStepper;
  //delete m_SagittalStepper;
  //delete m_FrontalStepper;
  //delete m_TimeStepper;
}

void QmitkImageNavigatorView::CreateQtPartControl(QWidget *parent)
{

  // create GUI widgets
  m_Controls.setupUi(parent);
  m_MultiWidget = this->GetActiveStdMultiWidget();

  m_TransversalStepper = new QmitkStepperAdapter(m_Controls.m_SliceNavigatorTransversal, m_MultiWidget->mitkWidget1->GetSliceNavigationController()->GetSlice(), "sliceNavigatorTransversalFromSimpleExample");
  m_SagittalStepper = new QmitkStepperAdapter(m_Controls.m_SliceNavigatorSagittal, m_MultiWidget->mitkWidget2->GetSliceNavigationController()->GetSlice(), "sliceNavigatorSagittalFromSimpleExample");
  m_FrontalStepper = new QmitkStepperAdapter(m_Controls.m_SliceNavigatorFrontal, m_MultiWidget->mitkWidget3->GetSliceNavigationController()->GetSlice(), "sliceNavigatorFrontalFromSimpleExample");
  m_TimeStepper = new QmitkStepperAdapter(m_Controls.m_SliceNavigatorTime, m_MultiWidget->GetTimeNavigationController()->GetTime(), "sliceNavigatorTimeFromSimpleExample");
}

void QmitkImageNavigatorView::SetFocus ()
{

}

QmitkStdMultiWidget* QmitkImageNavigatorView::GetActiveStdMultiWidget()
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

int QmitkImageNavigatorView::GetSizeFlags(bool width)
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

int QmitkImageNavigatorView::ComputePreferredSize(bool width, int availableParallel, int availablePerpendicular, int preferredResult)
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

