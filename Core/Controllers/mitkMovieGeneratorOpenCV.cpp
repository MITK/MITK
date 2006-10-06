#include "mitkMovieGeneratorOpenCV.h"
#include <GL/gl.h>


mitk::MovieGeneratorOpenCV::MovieGeneratorOpenCV()
{
  m_dwRate = 20;
}


void mitk::MovieGeneratorOpenCV::SetFileName( const char *fileName )
{
  m_sFile = _T( fileName );

  if( _tcsstr( (char*)m_sFile, _T("avi") ) == NULL ) m_sFile += _T( ".avi" );

}

bool mitk::MovieGeneratorOpenCV::InitGenerator()
{
  if (m_renderer) m_renderer->MakeCurrent();  // take the correct viewport!
  GLint viewport[4]; // Where The Viewport Values Will Be Stored
  glGetIntegerv( GL_VIEWPORT, viewport ); // Retrieves The Viewport Values (X, Y, Width, Height)
  m_width = viewport[2];  m_width -= m_width % 4;
  m_height = viewport[3];  m_height -= m_height % 4;
  
  m_currentFrame = cvCreateImage(cvSize(m_width,m_height),8,3);
  m_aviWriter = cvCreateVideoWriter(m_sFile,-1,m_dwRate,cvSize(m_width,m_height));
  
  if(!m_aviWriter)
    return false;
  
  return true;
}


bool mitk::MovieGeneratorOpenCV::AddFrame( void *data )
{
  cvSetImageData(m_currentFrame,data,m_width*3);
  cvWriteFrame(m_aviWriter,m_currentFrame);

  return true;
}


bool mitk::MovieGeneratorOpenCV::TerminateGenerator()
{
  if (m_aviWriter) {
    cvReleaseVideoWriter(&m_aviWriter);
  }
  return true;
}
