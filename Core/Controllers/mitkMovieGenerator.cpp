#include "mitkMovieGenerator.h"
#include <GL/gl.h>

#ifdef WIN32    
	#include "mitkMovieGeneratorWin32.h"
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
      MovieGeneratorWin32::Pointer wp = MovieGeneratorWin32::New();
      return wp;
    #endif
  }
  smartPtr = rawPtr;
  if(rawPtr != NULL) rawPtr->UnRegister();
  return smartPtr;
}


bool mitk::MovieGenerator::WriteMovie()
{  
	bool ok = false;  
	if (m_stepper) {    
		m_stepper->First();    
		ok = InitGenerator();    
		if (!ok) return false;   
		int imgSize = 3 * m_width * m_height;    
		printf( "Video size = %i x %i\n", m_width, m_height );    
		GLbyte *data = new GLbyte[imgSize];    
		for (int i=0; i<m_stepper->GetSteps(); i++) {      
			if (m_renderer) m_renderer->MakeCurrent();      
			glReadPixels( 0, 0, m_width, m_height, GL_BGR, GL_UNSIGNED_BYTE, (void*)data );      
			AddFrame( data );      
			m_stepper->Next();    
		}    
		ok = TerminateGenerator();    
		delete[] data;  
	}  
	return ok;
}