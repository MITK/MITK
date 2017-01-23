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
#include "mitkScaleLegendAnnotation.h"
#include "mitkLayoutAnnotationRenderer.h"

#include "QmitkPAUSViewerView.h"

const std::string QmitkPAUSViewerView::VIEW_ID = "org.mitk.views.photoacoustics.pausviewer";

QmitkPAUSViewerView::QmitkPAUSViewerView() : m_PADataStorage(mitk::StandaloneDataStorage::New()), m_USDataStorage(mitk::StandaloneDataStorage::New()), m_UltrasoundReference(nullptr)
{
}

QmitkPAUSViewerView::~QmitkPAUSViewerView()
{
  if(m_UltrasoundReference != nullptr)
    *m_UltrasoundReference = nullptr;
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
  if (m_PARenderer == nullptr || /*m_PAOverlayController == nullptr||*/ m_USRenderer == nullptr /*|| m_USOverlayController == nullptr*/)
  {
    m_PARenderer = mitk::BaseRenderer::GetInstance(GetPARenderWindow());
    m_USRenderer = mitk::BaseRenderer::GetInstance(GetUSRenderWindow());
  }

  mitk::ScaleLegendAnnotation::Pointer scaleAnnotation = mitk::ScaleLegendAnnotation::New();
  scaleAnnotation->SetLeftAxisVisibility(true);
  scaleAnnotation->SetRightAxisVisibility(false);
  scaleAnnotation->SetRightAxisVisibility(false);
  scaleAnnotation->SetTopAxisVisibility(false);
  scaleAnnotation->SetCornerOffsetFactor(0);
  mitk::LayoutAnnotationRenderer::AddAnnotation(scaleAnnotation.GetPointer(), m_PARenderer, mitk::LayoutAnnotationRenderer::TopLeft, 5, 5, 1);
  mitk::LayoutAnnotationRenderer::AddAnnotation(scaleAnnotation.GetPointer(), m_USRenderer, mitk::LayoutAnnotationRenderer::TopLeft, 5, 5, 1);
}

void QmitkPAUSViewerView::RemoveOverlays()
{
 // m_PAOverlayManager->RemoveAllOverlays();
}

void QmitkPAUSViewerView::SetUltrasoundReference(QmitkPAUSViewerView** ultrasoundReference)
{
  m_UltrasoundReference = ultrasoundReference;
}