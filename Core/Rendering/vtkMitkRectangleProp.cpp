/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date: 2007-08-17 16:41:18 +0200 (Fr, 17 Aug 2007) $
Version:   $Revision: 11618 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "vtkMitkRectangleProp.h"
#include "vtkObjectFactory.h"

#include "mitkGL.h"

vtkStandardNewMacro(vtkMitkRectangleProp);

vtkMitkRectangleProp::vtkMitkRectangleProp()
{
}
vtkMitkRectangleProp::~vtkMitkRectangleProp()
{
}

double *vtkMitkRectangleProp::GetBounds()
{
  return NULL;
}

int vtkMitkRectangleProp::RenderOverlay(vtkViewport* viewport)
{
  int * i = m_RenderWindow->GetSize();

  Enable2DOpenGL();

  glEnable(GL_LINE_SMOOTH);
 
  GLfloat bbox[8] = {0.f , 0.f, (float)i[0], 0.f, (float)i[0], (float)i[1], 0.f, (float)i[1]};
  glColor3f(m_Color[0],m_Color[1],m_Color[2]);
   
  glLineWidth(5.0f);
  glBegin(GL_LINE_LOOP);
  for (int j = 0; j < 4; j++)
    glVertex2fv(&bbox[2*j]);
  glEnd ();
  glLineWidth(1.0f);

  glDisable(GL_LINE_SMOOTH);

  //glFlush(); // really necessary ?? //B/

  Disable2DOpenGL();

  return 1;
}
int vtkMitkRectangleProp::RenderTranslucentGeometry(vtkViewport* viewport)
{
  return 0;
}
int vtkMitkRectangleProp::RenderOpaqueGeometry(vtkViewport* viewport)
{
  return 0;
}

void vtkMitkRectangleProp::Enable2DOpenGL()
{
  GLint iViewport[4];  
   
  // Get a copy of the viewport  
  glGetIntegerv( GL_VIEWPORT, iViewport );  
   
  // Save a copy of the projection matrix so that we can restore it  
  // when it's time to do 3D rendering again.  
  glMatrixMode( GL_PROJECTION );  
  glPushMatrix();  
  glLoadIdentity();  
   
  // Set up the orthographic projection  
  glOrtho( iViewport[0], iViewport[0]+iViewport[2],  
  iViewport[1]+iViewport[3], iViewport[1], -1, 1 );  
  glMatrixMode( GL_MODELVIEW );  
  glPushMatrix();  
  glLoadIdentity();  
   
  // Make sure depth testing and lighting are disabled for 2D rendering until  
  // we are finished rendering in 2D  
  glPushAttrib( GL_DEPTH_BUFFER_BIT | GL_LIGHTING_BIT );  
  glDisable( GL_DEPTH_TEST );  
  glDisable( GL_LIGHTING );  
}

void vtkMitkRectangleProp::Disable2DOpenGL()
{
  glPopAttrib();  
  glMatrixMode( GL_PROJECTION );  
  glPopMatrix();  
  glMatrixMode( GL_MODELVIEW );  
  glPopMatrix(); 
}