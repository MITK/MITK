#include "mitkMovieGenerator.h"
#include <GL/gl.h>


bool mitk::MovieGenerator::WriteMovie()
{
  bool ok = false;
  if (m_stepper) {
    m_stepper->First();
    ok = InitGenerator();
    if (!ok) return false;
    int imgSize = 3 * m_width * m_height;
    printf( "Video size = %i x %i\n", m_width, m_height );
    BYTE *data = new BYTE[imgSize];
    for (int i=0; i<m_stepper->GetSteps(); i++) {
      if (m_renderer) m_renderer->MakeCurrent();
      glReadPixels( 0, 0, m_width, m_height, GL_BGR_EXT, GL_UNSIGNED_BYTE, (void*)data );
      AddFrame( data );
      m_stepper->Next();
    }
    ok = TerminateGenerator();
    delete[] data;
  }
  return ok;
}