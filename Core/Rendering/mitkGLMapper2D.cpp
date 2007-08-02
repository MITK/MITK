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


#include "mitkGL.h"
#include "mitkGLMapper2D.h"

//##ModelId=3E681470037E
mitk::GLMapper2D::GLMapper2D()
{
}


//##ModelId=3E681470039C
mitk::GLMapper2D::~GLMapper2D()
{
}

//##ModelId=3E67E9390346
void mitk::GLMapper2D::Paint(mitk::BaseRenderer *)
{
}

//##ModelId=3EF17AA10120
void mitk::GLMapper2D::ApplyProperties(mitk::BaseRenderer* renderer)
{
    float rgba[4]={1.0f,1.0f,1.0f,1.0f};
    // check for color prop and use it for rendering if it exists
    GetColor(rgba, renderer);
    // check for opacity prop and use it for rendering if it exists
    GetOpacity(rgba[3], renderer);
    
    glColor4fv(rgba);
}