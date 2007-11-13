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


#include "mitkNativeRenderWindowInteractor.h"
#include "mitkVtkRenderWindow.h"

#include <vtkRenderWindowInteractor.h>

mitk::NativeRenderWindowInteractor::NativeRenderWindowInteractor() : m_MitkRenderWindow(NULL), m_NativeVtkRenderWindowInteractor(NULL)
{
  m_NativeVtkRenderWindowInteractor = vtkRenderWindowInteractor::New();
}

mitk::NativeRenderWindowInteractor::~NativeRenderWindowInteractor() 
{
  m_NativeVtkRenderWindowInteractor->Delete();
}

void mitk::NativeRenderWindowInteractor::SetMitkRenderWindow(mitk::RenderWindow* renderwindow)
{
  m_MitkRenderWindow = renderwindow;
  if(m_MitkRenderWindow != NULL)
    m_NativeVtkRenderWindowInteractor->SetRenderWindow(m_MitkRenderWindow->GetVtkRenderWindow());
}

void mitk::NativeRenderWindowInteractor::Start()
{
  if(m_MitkRenderWindow != NULL)
    m_NativeVtkRenderWindowInteractor->Start();
}
