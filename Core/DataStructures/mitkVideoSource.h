#ifndef _mitk_Video_Source_h_
#define _mitk_Video_Source_h_

#include "mitkCommon.h"
#include <itkObject.h>
#include "itkObjectFactory.h"

namespace mitk
{
  class VideoSource //: public itk::Object
  {
    public:
      //mitkClassMacro( VideoSource, itk::Object );
      //itkNewMacro( Self );
      VideoSource();
	    virtual ~VideoSource();      

      virtual void FetchFrame();
      virtual unsigned char * GetVideoTexture();

      virtual void StartCapturing();
      virtual void StopCapturing();
      bool IsCapturingEnabled();

    protected:
     
      
  	  
      unsigned char * m_CurrentVideoTexture;
      int m_CaptureWidth, m_CaptureHeight;
      bool m_CapturingInProcess;
   
 
  };
}
#endif // Header