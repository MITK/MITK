/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
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
  class MITKEXT_CORE_EXPORT VideoSource : public itk::Object
  {
    public:
      mitkClassMacro( VideoSource, itk::Object );
      itkNewMacro( Self );
         
      
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

      int GetImageWidth(){return m_CaptureWidth;}
      int GetImageHeight(){return m_CaptureHeight;}

      void EnableRotation(bool enable= true)
      {m_RotationEnabled = enable;};

      void SetRotationAngle(double rotationAngle)
      {m_RotationAngle = rotationAngle;};
      
      double GetRotationAngle()
      {return m_RotationAngle;};

    protected:
      VideoSource();
      virtual ~VideoSource();  
      
      unsigned char * m_CurrentVideoTexture;
      int m_CaptureWidth, m_CaptureHeight;
      bool m_CapturingInProcess;
      
      /**
      * Angle for rotating the video image
      **/
      double m_RotationAngle;
      
      /**
      * Flag to enable or disable video rotation used for performance enhancement.
      **/
      bool m_RotationEnabled;
   
 
  };
}
#endif // Header


