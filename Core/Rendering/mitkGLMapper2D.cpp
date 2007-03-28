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
#include "mitkGLUT.h"

//##ModelId=3E681470037E
mitk::GLMapper2D::GLMapper2D()
{
  static bool glutInitialized = false;
  if ( ! glutInitialized )
  {
    // glutInit must be called on systems using freeglut 2.4 and
    // above. On other systems it doesn't hurt anyway...
    //
    // @todo: (maybe) on linux check for DISPLAY environment. Otherwise 
    // glutInit fails anyway.
    std::string dummy("dummy");
    const char* dummy_str = dummy.c_str();
    char** argv = (char**) &dummy_str;
    int argc = 1;
    glutInit( &argc, argv );
    glutInitialized = true;
  }
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

void mitk::GLMapper2D::WriteTextXY(float x, float y, const std::string & text, bool /*drawLabel*/)
{
  // This code is work in progress. It does not work correctly yet, because the size of the text can not be calculated in screen(pixel) coordinates this way
  //if (drawLabel == true)
  //{
  //  float savedColor[4]={1.0f,1.0f,1.0f,1.0f};
  //  glGetFloatv(GL_CURRENT_COLOR, savedColor);  
  //  int txtlen = glutBitmapLength(GLUT_BITMAP_HELVETICA_10, (const unsigned char*)text.c_str());
  //  
  //  glRasterPos2f ( x, y);
  //  glColor4f(0.8f, 0.8f, 0.8f, 0.5f); // use semitransparent white color for label
  //  glBegin(GL_QUADS);                  // draw a label behind the text to improve readability 
  //    glVertex3f(x - 2, y + 2, 0.0f);   // label goes from x-2 to x+textlength+2 and y+2 to y-fontheight-2
  //    glVertex3f(x + txtlen + 2, y + 2, 0.0f); 
  //    glVertex3f(x + txtlen + 2, y - 12.0, 0.0f); 
  //    glVertex3f(x - 2, y - 12.0, 0.0f); 
  //  glEnd( );
  //  glColor4fv(savedColor);
  //}
  /* draw text*/
  glRasterPos2f ( x, y);
  for (unsigned int i = 0; i < text.size(); i++)
    glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, text[i]);
}

