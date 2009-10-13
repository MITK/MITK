/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-05-12 19:56:03 +0200 (Di, 12 Mai 2009) $
Version:   $Revision: 17179 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/



#include "mitkCompositeMapper.h"


mitk::CompositeMapper::CompositeMapper()
{
  m_OdfMapper = mitk::OdfVtkMapper2D<float,QBALL_ODFSIZE>::New();
  m_ImgMapper = mitk::CopyImageMapper2D::New();
}

mitk::CompositeMapper::~CompositeMapper()
{
}

void mitk::CompositeMapper::Enable2DOpenGL()
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
  glOrtho( 
    iViewport[0], iViewport[0]+iViewport[2],  
    iViewport[1], iViewport[1]+iViewport[3],
    -1.0, 1.0 
  );
  glMatrixMode( GL_MODELVIEW );  
  glPushMatrix();  
  glLoadIdentity();  
   
  // Make sure depth testing and lighting are disabled for 2D rendering until  
  // we are finished rendering in 2D  
  glPushAttrib( GL_DEPTH_BUFFER_BIT | GL_LIGHTING_BIT );  
  glDisable( GL_DEPTH_TEST );  
  glDisable( GL_LIGHTING );  
}

void mitk::CompositeMapper::Disable2DOpenGL()
{
  glPopAttrib();  
  glMatrixMode( GL_PROJECTION );  
  glPopMatrix();  
  glMatrixMode( GL_MODELVIEW );  
  glPopMatrix(); 
}
