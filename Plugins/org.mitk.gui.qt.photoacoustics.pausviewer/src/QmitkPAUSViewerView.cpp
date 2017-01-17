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

// Blueberry
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>
#include "mitkScaleLegendOverlay.h"

#include "QmitkPAUSViewerView.h"

const std::string QmitkPAUSViewerView::VIEW_ID = "org.mitk.views.photoacoustics.pausviewer";

QmitkPAUSViewerView::QmitkPAUSViewerView(): m_PADataStorage(mitk::StandaloneDataStorage::New()), m_USDataStorage(mitk::StandaloneDataStorage::New())
{
}

QmitkPAUSViewerView::~QmitkPAUSViewerView()
{
}

void QmitkPAUSViewerView::InitWindows()
{
  AddOverlays();
}

void QmitkPAUSViewerView::SetFocus()
{
}

void QmitkPAUSViewerView::OnSelectionChanged(berry::IWorkbenchPart::Pointer /*source*/,
  const QList<mitk::DataNode::Pointer>& nodes)
{
}

void QmitkPAUSViewerView::CreateQtPartControl(QWidget *parent)
{
  m_Controls = new Ui::QmitkPAUSViewerViewControls;
  m_Controls->setupUi(parent);

  m_Controls->m_PARenderWindow->GetRenderer()->SetDataStorage(m_PADataStorage);
  m_Controls->m_USRenderWindow->GetRenderer()->SetDataStorage(m_USDataStorage);
}

void QmitkPAUSViewerView::SetPADataStorage(mitk::StandaloneDataStorage::Pointer paStore)
{
  if (m_Controls == nullptr)
    return;

  m_PADataStorage = paStore;
  m_Controls->m_PARenderWindow->GetRenderer()->SetDataStorage(m_PADataStorage);
  m_Controls->m_PALevelWindow->SetDataStorage(m_PADataStorage);
}

void QmitkPAUSViewerView::SetUSDataStorage(mitk::StandaloneDataStorage::Pointer usStore)
{
  if (m_Controls == nullptr)
    return;

  m_USDataStorage = usStore;
  m_Controls->m_USRenderWindow->GetRenderer()->SetDataStorage(m_USDataStorage);
  m_Controls->m_USLevelWindow->SetDataStorage(m_USDataStorage);
}

vtkRenderWindow* QmitkPAUSViewerView::GetPARenderWindow()
{
  if (m_Controls == nullptr)
    return nullptr;

  return m_Controls->m_PARenderWindow->GetRenderWindow();
}

vtkRenderWindow* QmitkPAUSViewerView::GetUSRenderWindow()
{
  if (m_Controls == nullptr)
    return nullptr;

  return m_Controls->m_USRenderWindow->GetRenderWindow();
}


void QmitkPAUSViewerView::AddOverlays()
{
  if (m_PARenderer == nullptr || m_PAOverlayManager == nullptr || m_USRenderer == nullptr || m_USOverlayManager == nullptr)
  {
    //setup an overlay manager
    mitk::OverlayManager::Pointer OverlayManagerInstancePA = mitk::OverlayManager::New();
    m_PARenderer = mitk::BaseRenderer::GetInstance(GetPARenderWindow());
    m_PARenderer->SetOverlayManager(OverlayManagerInstancePA);
    m_PAOverlayManager = m_PARenderer->GetOverlayManager();

    mitk::OverlayManager::Pointer OverlayManagerInstanceUS = mitk::OverlayManager::New();
    m_USRenderer = mitk::BaseRenderer::GetInstance(GetUSRenderWindow());
    m_USRenderer->SetOverlayManager(OverlayManagerInstanceUS);
    m_USOverlayManager = m_USRenderer->GetOverlayManager();
  }

  mitk::ScaleLegendOverlay::Pointer scaleOverlay = mitk::ScaleLegendOverlay::New();
  scaleOverlay->SetLeftAxisVisibility(true);
  scaleOverlay->SetRightAxisVisibility(false);
  scaleOverlay->SetRightAxisVisibility(false);
  scaleOverlay->SetTopAxisVisibility(false);
  scaleOverlay->SetCornerOffsetFactor(0);
  m_PAOverlayManager->AddOverlay(scaleOverlay.GetPointer());
  m_USOverlayManager->AddOverlay(scaleOverlay.GetPointer());
}

void QmitkPAUSViewerView::RemoveOverlays()
{
  m_PAOverlayManager->RemoveAllOverlays();
}