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


#include "mitkGL.h"
#include "mitkGLMapper.h"


mitk::GLMapper::GLMapper()
{

}

mitk::GLMapper::~GLMapper()
{
}

void mitk::GLMapper::MitkRender(mitk::BaseRenderer* renderer, mitk::VtkPropRenderer::RenderType type){

  if(IsVisible(renderer)==false)
    return;

  Paint(renderer);
}


void mitk::GLMapper::ApplyProperties(mitk::BaseRenderer* renderer)
{
    float rgba[4]={1.0f,1.0f,1.0f,1.0f};
    // check for color prop and use it for rendering if it exists
    GetColor(rgba, renderer);
    // check for opacity prop and use it for rendering if it exists
    GetOpacity(rgba[3], renderer);

    glColor4fv(rgba);
}
