/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkRenderWindow.h"

#include "mitkVtkEventProvider.h"
#include "mitkVtkLayerController.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkRenderer.h"

mitk::RenderWindow::RenderWindow(vtkRenderWindow *renWin, const char *name)
  : m_vtkRenderWindow(renWin),
    m_vtkRenderWindowInteractor(nullptr),
    m_vtkMitkEventProvider(nullptr)
{
  if (nullptr == m_vtkRenderWindow)
  {
    m_vtkRenderWindow = vtkRenderWindow::New();
    m_vtkRenderWindow->SetMultiSamples(0); // We do not support MSAA as it is incompatible with depth peeling
    m_vtkRenderWindow->SetAlphaBitPlanes(1); // Necessary for depth peeling
  }

  if (m_vtkRenderWindow->GetSize()[0] <= 10)
  {
    m_vtkRenderWindow->SetSize(100, 100);
  }

  m_vtkRenderWindowInteractor = vtkRenderWindowInteractor::New();
  m_vtkRenderWindowInteractor->SetRenderWindow(m_vtkRenderWindow);
  m_vtkRenderWindowInteractor->Initialize();

  // initialize from RenderWindowBase
  this->Initialize(name);

  m_vtkMitkEventProvider = vtkEventProvider::New();
  m_vtkMitkEventProvider->SetInteractor(this->GetVtkRenderWindowInteractor());
  m_vtkMitkEventProvider->SetMitkRenderWindow(this);
  m_vtkMitkEventProvider->SetEnabled(1);
}

mitk::RenderWindow::~RenderWindow()
{
  Destroy();
  m_vtkRenderWindow->Delete();
  m_vtkRenderWindowInteractor->Delete();
  m_vtkMitkEventProvider->Delete();
}

vtkRenderWindow *mitk::RenderWindow::GetVtkRenderWindow()
{
  return m_vtkRenderWindow;
}

vtkRenderWindowInteractor *mitk::RenderWindow::GetVtkRenderWindowInteractor()
{
  return m_vtkRenderWindowInteractor;
}

void mitk::RenderWindow::SetSize(int width, int height)
{
  this->GetVtkRenderWindow()->SetSize(width, height);
}

void mitk::RenderWindow::ReinitEventProvider()
{
  m_vtkMitkEventProvider->SetEnabled(0);
  m_vtkMitkEventProvider->SetInteractor(this->GetVtkRenderWindowInteractor());
  m_vtkMitkEventProvider->SetMitkRenderWindow(this);
  m_vtkMitkEventProvider->SetEnabled(1);
}
