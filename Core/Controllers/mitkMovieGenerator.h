#ifndef	MOVIEGENERATOR_H_HEADER_INCLUDED

#define	MOVIEGENERATOR_H_HEADER_INCLUDED


#include "mitkCommon.h"
#include "mitkStepper.h"
#include "mitkBaseRenderer.h"

namespace mitk {  
  
class	MovieGenerator : public	itk::LightObject  { 

  public:    
    
    mitkClassMacro(MovieGenerator, itk::LightObject);    
    
    // delivers Win32 or Linux-versions of MovieGenerator    
    static Pointer New(void);    
    
    //!	stepper	used to	control	movie	generation    
    virtual void SetStepper( Stepper *stepper ) { m_stepper = stepper; }    
    
    //!	renderer to record    
    virtual void SetRenderer(	BaseRenderer *renderer ) { m_renderer	=	renderer;	}    
    
    //!	filename under which movie is	saved    
    virtual void SetFileName(	const	char *fileName ) { strcpy( m_fileName, fileName	); }    
    
    //!	uses given stepper and filename	to create a movie from the active OpenGL context    
    virtual bool WriteMovie();

  protected:

    //!	default	constructor
    MovieGenerator() { 
      m_fileName[0]	=	0;
      m_stepper	=	0;
      m_renderer = 0;
      m_width	=	0;
      m_height = 0;
    }

    //!	called directly	before the first frame is	added, determines	movie	size from	renderer
    virtual	bool InitGenerator() = 0;

    //!	used to	add	a	frame
    virtual	bool AddFrame( void	*data	)	=	0;

    //!	called after the last	frame	is added
    virtual	bool TerminateGenerator()	=	0;


    //!	stores the movie filename
    char				 m_fileName[1024];

    //!	stores the used	stepper
    Stepper			 *m_stepper;

    //! stores the uses renderer
    BaseRenderer *m_renderer;

    //!	InitGenerator()	stores movie size	in those variables
    int					 m_width,	m_height;  

};

} //namespace mitk

#endif /*	MOVIEGENERATOR_H_HEADER_INCLUDED */

