/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef _mitk_Video_Source_h_
#define _mitk_Video_Source_h_

#include "mitkCommon.h"
#include "MitkExtExports.h"
#include <itkObject.h>
#include <itkObjectFactory.h>
#include <mitkMessage.h>

namespace mitk
{
  ///
  /// Simple base class for acquiring video data.
  ///
  class MitkExt_EXPORT VideoSource: virtual public itk::Object
  {
    public:
      ///
      /// Smart pointer defs
      ///
      mitkClassMacro( VideoSource, itk::Object );
      ///
      /// assigns the grabbing devices for acquiring the next frame.
      /// in this base implementation it does nothing except incrementing
      /// m_FrameCount
      ///
      virtual void FetchFrame();
      ///
      /// \return a pointer to the image data array for opengl rendering.
      ///
      virtual unsigned char * GetVideoTexture() = 0;
      ///
      /// advices this class to start the video capturing.
      /// in this base implementation: toggles m_CapturingInProcess, resets m_FrameCount
      /// *ATTENTION*: this should be also done in subclasses overwriting this method
      ///
      virtual void StartCapturing();
      ///
      /// advices this class to stop the video capturing.
      /// in this base implementation: toggles m_CapturingInProcess, resets m_FrameCount
      /// *ATTENTION*: this should be also done in subclasses overwriting this method
      ///
      virtual void StopCapturing();
      ///
      /// \return true if video capturing is active.
      /// \see m_CapturingInProcess
      ///
      virtual bool IsCapturingEnabled() const;
      ///
      /// \return the current frame width (might be 0 if unknown)
      ///
      virtual int GetImageWidth();
      ///
      /// \return the current frame height (might be 0 if unknown)
      ///
      virtual int GetImageHeight();
      ///
      /// \return the current frame count
      ///
      virtual unsigned long GetFrameCount() const;
      ///
      /// \return true, if capturing is currently paused, false otherwise
      ///
      virtual bool GetCapturePaused() const;
      ///
      /// toggles m_CapturePaused
      /// In Subclasses this function can be overwritten to take
      /// measurs to provide a pause image, *BUT DO NOT FORGET TO
      /// TOGGLE m_CapturePaused*
      ///
      virtual void PauseCapturing();

  protected:
      ///
      /// init member
      ///
      VideoSource();
      ///
      /// deletes m_CurrentVideoTexture (if not 0)
      ///
      virtual ~VideoSource();

  protected:
      ///
      /// finally this is what the video source must create: a video texture pointer
      /// default: 0
      ///
      unsigned char * m_CurrentVideoTexture;

      ///
      /// should be filled when the first frame is available
      /// default: 0
      ///
      int m_CaptureWidth;

      ///
      /// should be filled when the first frame is available
      /// default: 0
      ///
      int m_CaptureHeight;

      ///
      /// saves if capturing is in procress
      /// default: false
      ///
      bool m_CapturingInProcess;

      ///
      /// Saves the current frame count. Incremented in FetchFrame().
      /// Resetted to 0 when StartCapturing() or StopCapturing() is called.
      /// default: 0
      ///
      unsigned long m_FrameCount;

      ///
      /// Saves if the capturing is currently paused, i.e. this
      /// will not fetch any further frames but provide the current
      /// frame as long as m_CapturePaused is true
      /// default: false
      ///
      bool m_CapturePaused;

  };
}
#endif // Header


