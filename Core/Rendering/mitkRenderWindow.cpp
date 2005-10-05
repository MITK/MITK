/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
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


#include "mitkRenderWindow.h"
#include "mitkVtkRenderWindow.h"
#include "mitkOpenGLRenderer.h"
#include "mitkRenderingManager.h"
#include <assert.h>

std::set<mitk::RenderWindow*> mitk::RenderWindow::instances;

mitk::RenderWindow::RenderWindow(const char *name, mitk::BaseRenderer* renderer) 
  : m_MitkVtkRenderWindow(NULL), m_Renderer(renderer)
{
  if(name == NULL)
    m_Name = "renderwindow";
  else
    m_Name = name;
  instances.insert(this);

  RenderingManager::GetInstance()->AddRenderWindow( this );

  m_MitkVtkRenderWindow = mitk::VtkRenderWindow::New();
}

mitk::RenderWindow::~RenderWindow()
{
  instances.erase(this);

  RenderingManager::GetInstance()->RemoveRenderWindow( this );

  m_Renderer->InitRenderer(NULL);
  m_Renderer = NULL;

  m_MitkVtkRenderWindow->Delete(); //xxx
}

void mitk::RenderWindow::MakeCurrent() 
{
  m_MitkVtkRenderWindow->MakeCurrent();
};

void mitk::RenderWindow::SwapBuffers() 
{
  m_MitkVtkRenderWindow->Frame();
};

bool mitk::RenderWindow::IsSharing () const
{
  return false;
}

void mitk::RenderWindow::RequestUpdate()
{
  mitk::RenderingManager::GetInstance()->RequestUpdate( this );
}

void mitk::RenderWindow::ForceImmediateUpdate()
{
  mitk::RenderingManager::GetInstance()->ForceImmediateUpdate( this );
}

void mitk::RenderWindow::SetSize(int w, int h)
{
  m_MitkVtkRenderWindow->SetSize(w,h);
}

void mitk::RenderWindow::InitRenderer()
{
  if(m_Renderer.IsNull())
  {
    std::string rendererName("Renderer::");
    rendererName += m_Name;
    m_Renderer = new OpenGLRenderer( rendererName.c_str() );
  }

  m_MitkVtkRenderWindow->SetMitkRenderer(m_Renderer);

  m_Renderer->InitRenderer(this);

  int * size = m_MitkVtkRenderWindow->GetSize();
  if((size[0]>10) && (size[1]>10))
    m_Renderer->InitSize(size[0], size[1]);
}

void mitk::RenderWindow::SetWindowId(void * id)
{
  m_MitkVtkRenderWindow->SetWindowId( id );
}

void mitk::RenderWindow::SetVtkRenderWindow(VtkRenderWindow* renWin)
{
  if (renWin != NULL)
  {
    m_MitkVtkRenderWindow = renWin;
    InitRenderer();
  }
}


