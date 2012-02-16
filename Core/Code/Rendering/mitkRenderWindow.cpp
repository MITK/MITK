/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2010-07-05 09:49:37 +0200 (Mo, 05 Jul 2010) $
Version:   $Revision: 24298 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#include "mitkRenderWindow.h"

#include "mitkDisplayPositionEvent.h"
#include "mitkVtkLayerController.h"
#include "mitkRenderingManager.h"
#include "vtkRenderer.h"
#include "vtkRenderWindowInteractor.h"
#include "mitkVtkEventProvider.h"


mitk::RenderWindow::RenderWindow(vtkRenderWindow* renWin, const char* name, mitk::RenderingManager* rm ) 
: m_vtkRenderWindow(renWin)
{
  
  if(m_vtkRenderWindow == NULL)
    m_vtkRenderWindow = vtkRenderWindow::New();

  if ( m_vtkRenderWindow->GetSize()[0] > 10 
    || m_vtkRenderWindow->GetSize()[0] > 10 )
  {
    m_vtkRenderWindow->SetSize( 100, 100 );
  }
 
  m_vtkRenderWindowInteractor = vtkRenderWindowInteractor::New();
  m_vtkRenderWindowInteractor->SetRenderWindow(m_vtkRenderWindow);
  m_vtkRenderWindowInteractor->Initialize();

  // initialize from RenderWindowBase
  Initialize(rm,name);

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

vtkRenderWindow* mitk::RenderWindow::GetVtkRenderWindow()
{
  return m_vtkRenderWindow;
}

vtkRenderWindowInteractor* mitk::RenderWindow::GetVtkRenderWindowInteractor()
{
  return m_vtkRenderWindowInteractor;   
}


void mitk::RenderWindow::SetSize( int width, int height )
{
  this->GetVtkRenderWindow()->SetSize( width, height );

  this->resizeMitkEvent( width, height );
}

void mitk::RenderWindow::ReinitEventProvider()
{
  m_vtkMitkEventProvider->SetEnabled(0);
  m_vtkMitkEventProvider->SetInteractor(this->GetVtkRenderWindowInteractor());
  m_vtkMitkEventProvider->SetMitkRenderWindow(this);
  m_vtkMitkEventProvider->SetEnabled(1);
}

