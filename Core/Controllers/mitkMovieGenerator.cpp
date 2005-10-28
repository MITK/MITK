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


#include "mitkMovieGenerator.h"
#include <mitkRenderingManager.h>
#include <GL/gl.h>

#ifdef WIN32
#ifndef __GNUC__
#include "mitkMovieGeneratorWin32.h"
#endif
#endif
#ifndef GL_BGR
#define GL_BGR GL_BGR_EXT
#endif

mitk::MovieGenerator::Pointer mitk::MovieGenerator::New() 
{
  Pointer smartPtr;
  MovieGenerator *rawPtr = ::itk::ObjectFactory<MovieGenerator>::Create();
  if(rawPtr == NULL) {
#ifdef WIN32
#ifndef __GNUC__
    mitk::MovieGenerator::Pointer wp = static_cast<mitk::MovieGenerator*>(MovieGeneratorWin32::New());
    return wp;
#endif
#endif
  }
  smartPtr = rawPtr;
  if(rawPtr != NULL) rawPtr->UnRegister();
  return smartPtr;
}


bool mitk::MovieGenerator::WriteMovie()
{  
  bool ok = false;
  if (m_stepper)
  {
    if (m_renderer) m_renderer->MakeCurrent();
    m_stepper->First();
    RenderingManager::GetInstance()->ForceImmediateUpdate(m_renderer->GetRenderWindow());

    ok = InitGenerator();
    if (!ok) 
    {
      TerminateGenerator();
      return false;
    }
    int imgSize = 3 * m_width * m_height;
    printf( "Video size = %i x %i\n", m_width, m_height );
    GLbyte *data = new GLbyte[imgSize];
    for (unsigned int i=0; i<m_stepper->GetSteps(); i++)
    {
      if (m_renderer) m_renderer->MakeCurrent();
      RenderingManager::GetInstance()->ForceImmediateUpdate(m_renderer->GetRenderWindow());
      glReadPixels( 0, 0, m_width, m_height, GL_BGR, GL_UNSIGNED_BYTE, (void*)data );
      AddFrame( data );
      m_stepper->Next();
    }
    ok = TerminateGenerator();
    delete[] data;
  }
  return ok;
}
