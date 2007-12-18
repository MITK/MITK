/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
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


#include "mitkGL.h"
#include "mitkGLMapper2D.h"

mitk::GLMapper2D::GLMapper2D()
{
  m_VtkBased = false;
}


mitk::GLMapper2D::~GLMapper2D()
{
}

void mitk::GLMapper2D::MitkRenderOpaqueGeometry(mitk::BaseRenderer* renderer)
{
  if(IsVisible(renderer)==false) 
    return;

  Paint(renderer);  
}
void mitk::GLMapper2D::MitkRenderTranslucentGeometry(mitk::BaseRenderer* renderer)
{
 
}
void mitk::GLMapper2D::MitkRenderOverlay(mitk::BaseRenderer* renderer)
{
 
}

void mitk::GLMapper2D::ApplyProperties(mitk::BaseRenderer* renderer)
{
    float rgba[4]={1.0f,1.0f,1.0f,1.0f};
    // check for color prop and use it for rendering if it exists
    GetColor(rgba, renderer);
    // check for opacity prop and use it for rendering if it exists
    GetOpacity(rgba[3], renderer);
    
    glColor4fv(rgba);
}
