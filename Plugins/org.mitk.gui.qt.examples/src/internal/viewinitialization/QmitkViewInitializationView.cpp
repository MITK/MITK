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

#include "QmitkViewInitializationView.h"

#include "mitkNodePredicateDataType.h"

#include "QmitkDataStorageComboBox.h"
#include "QmitkStdMultiWidget.h"

#include "mitkFocusManager.h"
#include "mitkGlobalInteraction.h"
#include "itkCommand.h"


#include <QMessageBox>



const std::string QmitkViewInitializationView::VIEW_ID = "org.mitk.views.viewinitialization";

QmitkViewInitializationView::QmitkViewInitializationView()
: QmitkFunctionality(),
  m_Controls(NULL),
  m_MultiWidget(NULL)
{
  m_CommandTag = 0;
}

QmitkViewInitializationView::~QmitkViewInitializationView()
{
}

void QmitkViewInitializationView::CreateQtPartControl(QWidget *parent)
{
  if (!m_Controls)
  {
    // create GUI widgets
    m_Controls = new Ui::QmitkViewInitializationViewControls;
    m_Controls->setupUi(parent);
    this->CreateConnections();
  }
}

void QmitkViewInitializationView::StdMultiWidgetAvailable (QmitkStdMultiWidget &stdMultiWidget)
{
  m_MultiWidget = &stdMultiWidget;
}

void QmitkViewInitializationView::StdMultiWidgetNotAvailable()
{
  m_MultiWidget = NULL;
}

void QmitkViewInitializationView::CreateConnections()
{
  if ( m_Controls )
  {
    connect( (QObject*)(m_Controls->pbApply), SIGNAL(clicked()),(QObject*) this, SLOT(OnApply()) );
    connect( (QObject*)(m_Controls->pbReset), SIGNAL(clicked()),(QObject*) this, SLOT(OnResetAll()) );
  }
}

void QmitkViewInitializationView::Activated()
{
  //init render window selector (List Widget)
  this->InitRenderWindowSelector();
  QmitkFunctionality::Activated();
}

void QmitkViewInitializationView::Deactivated()
{
  mitk::FocusManager* fm = mitk::GlobalInteraction::GetInstance()->GetFocusManager();
  fm->RemoveObserver(m_CommandTag);
  QmitkFunctionality::Deactivated();
}


void QmitkViewInitializationView::OnApply()
{
  mitk::SliceNavigationController::ViewDirection viewDirection( mitk::SliceNavigationController::Axial );
  if( m_Controls->rbAxial->isChecked() )
    viewDirection = mitk::SliceNavigationController::Axial;

  else if( m_Controls->rbFrontal->isChecked())
    viewDirection = mitk::SliceNavigationController::Frontal;

  else if( m_Controls->rbSagittal->isChecked() )
    viewDirection = mitk::SliceNavigationController::Sagittal;

  vtkRenderWindow* renderwindow = this->GetSelectedRenderWindow();
  if(renderwindow != NULL)
  {
    mitk::BaseRenderer::GetInstance(renderwindow)->GetSliceNavigationController()->Update(viewDirection,
      m_Controls->cbTop->isChecked(),
      m_Controls->cbFrontSide->isChecked(),
      m_Controls->cbRotated->isChecked()
      );
    mitk::BaseRenderer::GetInstance(renderwindow)->GetDisplayGeometry()->Fit();
  }
}

void QmitkViewInitializationView::OnResetAll()
{
  /* calculate bounding geometry of these nodes */
  mitk::TimeGeometry::Pointer bounds = this->GetDefaultDataStorage()->ComputeBoundingGeometry3D();
  /* initialize the views to the bounding geometry */
  mitk::RenderingManager::GetInstance()->InitializeViews(bounds);
}

vtkRenderWindow* QmitkViewInitializationView::GetSelectedRenderWindow()
{
  int selectedItem = m_Controls->m_lbRenderWindows->currentRow();
  int itemNumber = 0;

  mitk::BaseRenderer::BaseRendererMapType::iterator mapit;
  for(mapit = mitk::BaseRenderer::baseRendererMap.begin();
    mapit != mitk::BaseRenderer::baseRendererMap.end();
    mapit++, itemNumber++)
  {
    if(itemNumber==selectedItem)
      break;
  }
   if(itemNumber==selectedItem)
  {
    return (*mapit).first;
  }
  return NULL;
}

void QmitkViewInitializationView::InitRenderWindowSelector()
{
  itk::SimpleMemberCommand<QmitkViewInitializationView>::Pointer updateRendererListCommand =
    itk::SimpleMemberCommand<QmitkViewInitializationView>::New();
  updateRendererListCommand->SetCallbackFunction( this, &QmitkViewInitializationView::UpdateRendererList );

  mitk::FocusManager* fm = mitk::GlobalInteraction::GetInstance()->GetFocusManager();
  m_CommandTag = fm->AddObserver(mitk::FocusEvent(), updateRendererListCommand);

  this->UpdateRendererList();
}

void QmitkViewInitializationView::UpdateRendererList()
{
  vtkRenderWindow* focusedRenderWindow = NULL;

  mitk::FocusManager* fm = mitk::GlobalInteraction::GetInstance()->GetFocusManager();

  mitk::BaseRenderer::ConstPointer br = fm->GetFocused();

  if (br.IsNotNull())
  {
    focusedRenderWindow = br->GetRenderWindow();
  }

  int selectedItem = -1;
  int itemNumber = 0;
  m_Controls->m_lbRenderWindows->clear();


  for(mitk::BaseRenderer::BaseRendererMapType::iterator mapit = mitk::BaseRenderer::baseRendererMap.begin();
    mapit != mitk::BaseRenderer::baseRendererMap.end(); mapit++, itemNumber++)
  {
    if( (*mapit).second->GetName())
    {
      m_Controls->m_lbRenderWindows->addItem(QString((*mapit).second->GetName()));
      if(focusedRenderWindow==(*mapit).first)
        selectedItem = itemNumber;
    }
  }

  if (selectedItem>=0)
  {
    m_Controls->m_lbRenderWindows->setCurrentRow(selectedItem);
  }
  else
  {
    m_Controls->m_lbRenderWindows->clearSelection();
  }
}
