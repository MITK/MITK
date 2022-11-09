/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkViewInitializationView.h"

#include "mitkNodePredicateDataType.h"

#include "QmitkDataStorageComboBox.h"
#include "mitkCameraController.h"
#include <mitkBaseRenderer.h>
#include <mitkSliceNavigationController.h>

#include "itkCommand.h"

#include <QMessageBox>

const std::string QmitkViewInitializationView::VIEW_ID = "org.mitk.views.viewinitialization";

QmitkViewInitializationView::QmitkViewInitializationView() : m_Controls(nullptr)
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

void QmitkViewInitializationView::SetFocus()
{
  m_Controls->pbApply->setFocus();
}

void QmitkViewInitializationView::CreateConnections()
{
  if (m_Controls)
  {
    connect((QObject *)(m_Controls->pbApply), SIGNAL(clicked()), (QObject *)this, SLOT(OnApply()));
    connect((QObject *)(m_Controls->pbReset), SIGNAL(clicked()), (QObject *)this, SLOT(OnResetAll()));
  }
}

void QmitkViewInitializationView::Activated()
{
  // init render window selector (List Widget)
  this->InitRenderWindowSelector();
}

void QmitkViewInitializationView::Deactivated()
{
}

void QmitkViewInitializationView::Visible()
{
}

void QmitkViewInitializationView::Hidden()
{
}

void QmitkViewInitializationView::OnApply()
{
  mitk::AnatomicalPlane anatomicalPlane(mitk::AnatomicalPlane::Axial);
  if (m_Controls->rbAxial->isChecked())
    anatomicalPlane = mitk::AnatomicalPlane::Axial;

  else if (m_Controls->rbCoronal->isChecked())
    anatomicalPlane = mitk::AnatomicalPlane::Coronal;

  else if (m_Controls->rbSagittal->isChecked())
    anatomicalPlane = mitk::AnatomicalPlane::Sagittal;

  vtkRenderWindow *renderwindow = this->GetSelectedRenderWindow();
  if (renderwindow != nullptr)
  {
    mitk::BaseRenderer::GetInstance(renderwindow)
      ->GetSliceNavigationController()
      ->Update(anatomicalPlane,
               m_Controls->cbTop->isChecked(),
               m_Controls->cbFrontSide->isChecked(),
               m_Controls->cbRotated->isChecked());
    mitk::BaseRenderer::GetInstance(renderwindow)->GetCameraController()->Fit();
  }
}

void QmitkViewInitializationView::OnResetAll()
{
  /* calculate bounding geometry of these nodes */
  auto bounds = this->GetDataStorage()->ComputeBoundingGeometry3D();
  /* initialize the views to the bounding geometry */
  mitk::RenderingManager::GetInstance()->InitializeViews(bounds);
}

vtkRenderWindow *QmitkViewInitializationView::GetSelectedRenderWindow()
{
  int selectedItem = m_Controls->m_lbRenderWindows->currentRow();
  int itemNumber = 0;

  mitk::BaseRenderer::BaseRendererMapType::iterator mapit;
  for (mapit = mitk::BaseRenderer::baseRendererMap.begin(); mapit != mitk::BaseRenderer::baseRendererMap.end();
       mapit++, itemNumber++)
  {
    if (itemNumber == selectedItem)
      break;
  }
  if (itemNumber == selectedItem)
  {
    return (*mapit).first;
  }
  return nullptr;
}

void QmitkViewInitializationView::InitRenderWindowSelector()
{
  itk::SimpleMemberCommand<QmitkViewInitializationView>::Pointer updateRendererListCommand =
    itk::SimpleMemberCommand<QmitkViewInitializationView>::New();
  updateRendererListCommand->SetCallbackFunction(this, &QmitkViewInitializationView::UpdateRendererList);

  this->UpdateRendererList();
}

void QmitkViewInitializationView::UpdateRendererList()
{
  vtkRenderWindow *focusedRenderWindow = mitk::RenderingManager::GetInstance()->GetFocusedRenderWindow();

  int selectedItem = -1;
  int itemNumber = 0;
  m_Controls->m_lbRenderWindows->clear();

  for (mitk::BaseRenderer::BaseRendererMapType::iterator mapit = mitk::BaseRenderer::baseRendererMap.begin();
       mapit != mitk::BaseRenderer::baseRendererMap.end();
       mapit++, itemNumber++)
  {
    if ((*mapit).second->GetName())
    {
      m_Controls->m_lbRenderWindows->addItem(QString((*mapit).second->GetName()));
      if (focusedRenderWindow == (*mapit).first)
        selectedItem = itemNumber;
    }
  }

  if (selectedItem >= 0)
  {
    m_Controls->m_lbRenderWindows->setCurrentRow(selectedItem);
  }
  else
  {
    m_Controls->m_lbRenderWindows->clearSelection();
  }
}
