#ifndef MovieGeneratorOpenCV_H_HEADER_INCLUDED
#define MovieGeneratorOpenCV_H_HEADER_INCLUDED

#include "mitkMovieGenerator.h"
#include <comdef.h>
#include <memory.h>
#include <tchar.h>
#include <string.h>

// OpenCV includes
#include "cv.h"
#include "highgui.h"


namespace mitk {


class MovieGeneratorOpenCV : public MovieGenerator
{

public:

  mitkClassMacro(MovieGeneratorOpenCV, MovieGenerator);
  itkNewMacro(Self);

  virtual void SetFileName( const char *fileName );


protected:

  MovieGeneratorOpenCV();

  //! called directly before the first frame is added
  virtual bool InitGenerator();

  //! used to add a frame
  virtual bool AddFrame( void *data );

  //! called after the last frame is added
  virtual bool TerminateGenerator();

  //! name of output file
  _bstr_t m_sFile;

  //! frame rate 
  DWORD m_dwRate;

  
private:
  
  CvVideoWriter*    m_aviWriter;
  IplImage *        m_currentFrame;

  //! frame counter
  long m_lFrame;
};

} // namespace mitk

#endif /* MovieGeneratorOpenCV_H_HEADER_INCLUDED */
