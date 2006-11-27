#ifndef _mitk_Video_Source_h_
#define _mitk_Video_Source_h_

#include "mitkCommon.h"
#include <itkObject.h>
#include "itkObjectFactory.h"

namespace mitk
{
  /**
  * Simple base class for acquiring video data. 
  */
  class VideoSource //: public itk::Object
  {
    public:
      //mitkClassMacro( VideoSource, itk::Object );
      //itkNewMacro( Self );
      VideoSource();
	    virtual ~VideoSource();      
      
      ////##Documentation
      ////## @brief assigns the grabbing devices for acquiring the next frame. 
      virtual void FetchFrame();
      ////##Documentation
      ////## @brief returns a pointer to the image data array for opengl rendering. 
      virtual unsigned char * GetVideoTexture();
      ////##Documentation
      ////## @brief starts the video capturing.
      virtual void StartCapturing();
      ////##Documentation
      ////## @brief stops the video capturing.
      virtual void StopCapturing();
      ////##Documentation
      ////## @brief returns true if video capturing is active.
      bool IsCapturingEnabled();

    protected:
  	  
      unsigned char * m_CurrentVideoTexture;
      int m_CaptureWidth, m_CaptureHeight;
      bool m_CapturingInProcess;
   
 
  };
}
#endif // Header