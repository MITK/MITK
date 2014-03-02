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


#include "mitkNativeRenderWindowInteractor.h"

#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>

mitk::NativeRenderWindowInteractor::NativeRenderWindowInteractor() : m_MitkRenderWindow(NULL), m_NativeVtkRenderWindowInteractor(NULL)
{
  m_NativeVtkRenderWindowInteractor = vtkRenderWindowInteractor::New();
}

mitk::NativeRenderWindowInteractor::~NativeRenderWindowInteractor()
{
  m_NativeVtkRenderWindowInteractor->Delete();
}

void mitk::NativeRenderWindowInteractor::SetMitkRenderWindow(vtkRenderWindow* renderwindow)
{
  m_MitkRenderWindow = renderwindow;
  if(m_MitkRenderWindow != NULL)
    m_NativeVtkRenderWindowInteractor->SetRenderWindow(m_MitkRenderWindow);
}

void mitk::NativeRenderWindowInteractor::Start()
{
  if(m_MitkRenderWindow != NULL)
    m_NativeVtkRenderWindowInteractor->Start();
}
