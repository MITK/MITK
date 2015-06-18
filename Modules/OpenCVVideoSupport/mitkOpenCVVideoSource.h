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

#ifndef _mitk_OpenCVVideo_Source_h_
#define _mitk_OpenCVVideo_Source_h_

#include "mitkConfig.h"

#include "mitkVideoSource.h"
#include "mitkUndistortCameraImage.h"

// HighGui camera interface: a convenient way for grabbing from a video capture (on windows VfW is used)
#include "highgui.h"

// For Providing ITK Image Interface
#include "itkRGBPixel.h"
#include "itkImage.h"
#include "itkImageRegionIterator.h"
#include "mitkOpenCVImageSource.h"

namespace mitk
{
 /**
 * Interface for acquiring video data using Intel's OPENCV library.
 * Video data may either be provided from a file or a grabbing device.
 * At the moment, OPENCV includes two separated modules for this grabbing, but only HighGui is
 * used here.
 * Initialize via SetVideoFileInput() or SetVideoCameraInput(), start processing with StartCapturing();
 */
  class MITKOPENCVVIDEOSUPPORT_EXPORT OpenCVVideoSource :
      virtual public VideoSource, virtual public OpenCVImageSource
  {
  public:
    typedef itk::RGBPixel< unsigned char >    CharPixelType;
    typedef itk::FixedArray <float,3>         HSVPixelType;
    typedef itk::Image< CharPixelType , 2 >   RGBPixelImageType;
    typedef itk::Image<HSVPixelType,2> HSVPixelImageType;
    typedef itk::ImageRegionIterator< RGBPixelImageType > RGBConstIteratorType;
    typedef itk::ImageRegionIterator< HSVPixelImageType > HSVConstIteratorType;

    mitkClassMacro( OpenCVVideoSource, VideoSource );
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    ////##Documentation
    ////## @brief sets a video file as input device. One video frame is being processed by updating the renderwindow.
    ////## Notice: Which codecs and file formats are supported depends on the back end library.
    ////## Common Function that currently uses HighGui Lib for video playback
    virtual void SetVideoFileInput(const char * filename, bool repeatVideo, bool useCVCAMLib = false);
    ////##Documentation
    ////##@brief Initializes capturing video from camera.
    ////## Common Function for use either with HIGHGUI or with CVCAM library
    ////## On windows: if you use CVCAM Library, you can pass -1 as camera index for a selection menu
    virtual void SetVideoCameraInput(int cameraindex, bool useCVCAMLib = false);
    ////##Documentation
    ////## The function GetVideoCaptureProperty retrieves the specified property of camera or video file from HIGHGUI LIBRARY.
    ////## Video input has to be initialized before call, that means: at least one frame has to be grabbed already.
    ////## The property_id identifier can be the following:
    ////## CV_CAP_PROP_POS_MSEC film current position in milliseconds or video capture timestamp
    ////## CV_CAP_PROP_POS_FRAMES 0-based index of the frame to be decoded/captured next
    ////## CV_CAP_PROP_POS_AVI_RATIO relative position of video file (0 - start of the film, 1 - end of the film)
    ////## CV_CAP_PROP_FRAME_WIDTH width of frames in the video stream
    ////## CV_CAP_PROP_FRAME_HEIGHT height of frames in the video stream
    ////## CV_CAP_PROP_FPS frame rate
    ////## CV_CAP_PROP_FOURCC 4-character code of codec
    ////## CV_CAP_PROP_FRAME_COUNT number of frames in video file
    ////## See OpenCV Highgui documentation for more details ( http://opencvlibrary.sourceforge.net/HighGui )
    virtual double GetVideoCaptureProperty(int property_id);
    ////##Documentation
    ////## @brief sets the specified property of video capturing from HIGHGUI LIBRARY.
    ////## Notice: Some properties only can be set using a video file as input devices, others using a camera.
    ////## See OpenCV Highgui documentation for more details ( http://opencvlibrary.sourceforge.net/HighGui )
    virtual int SetVideoCaptureProperty(int property_id, double value);

    virtual void GetCurrentFrameAsOpenCVImage(IplImage * image);
    ///
    /// \return a copy of the image as opencv 2 Mat
    ///
    virtual cv::Mat GetImage() override;
    virtual const IplImage * GetCurrentFrame();
    ////##Documentation
    ////## @brief returns the current video data as an ITK image.
    virtual void GetCurrentFrameAsItkHSVPixelImage(HSVPixelImageType::Pointer &Image);
    ////##Documentation
    ////## @brief assigns the grabbing devices for acquiring the next frame.
    virtual void FetchFrame() override;
    ////##Documentation
    ////## @brief returns a pointer to the image data array for opengl rendering.
    virtual unsigned char * GetVideoTexture() override;
    ////##Documentation
    ////## @brief starts the video capturing.
    virtual void StartCapturing() override;
    ////##Documentation
    ////## @brief stops the video capturing.
    virtual void StopCapturing() override;
    ////##Documentation
    ////## @brief rotate image according to the set angle.
    virtual IplImage* FlipImage(IplImage* input);
    ////##Documentation
    ////## @brief EnableOnlineImageUndistortion allows for an online image undistortion directly after capturing an image.
    ////## The function has to be called after setting up the video input; the result is made accessible via the normal
    ////## GetCurrentFrame... functions.
    virtual void EnableOnlineImageUndistortion(mitk::Point3D focal, mitk::Point3D principal, mitk::Point4D distortion);
     ////##Documentation
    ////## @brief DisableOnlineImageUndistortion is used to disable the automatic image undistortion.
    virtual void DisableOnlineImageUndistortion();
    ///
    /// \return true if image undistorsion is enabled
    ///
    virtual bool OnlineImageUndistortionEnabled() const;

    virtual void PauseCapturing() override;
    ///
    /// Returns the video file name (maybe empty if a grabbing device is used)
    ///
    itkGetConstMacro( VideoFileName, std::string );

    virtual void SetEnableXAxisFlip(bool enable);
    virtual void SetEnableYAxisFlip(bool enable);

    ///
    /// Returns the GrabbingDeviceNumber (maybe -1 if a video file is used)
    ///
    itkGetConstMacro( GrabbingDeviceNumber, short );

    itkGetMacro( RepeatVideo, bool );
    itkSetMacro( RepeatVideo, bool );


  protected:
    OpenCVVideoSource();
    virtual ~OpenCVVideoSource();

    ///
    /// Resets the whole class for capturing from a new device
    ///
    void Reset();

    ////##Documentation
    ////## @brief internally used for converting the current video frame to a texture for opengl rendering,
    ////## so that GetVideoTexture() can be used.
    void UpdateVideoTexture();

    // Helper functions
    void sleep(unsigned int ms);
    void RGBtoHSV(float r, float g, float b, float &h, float &s, float &v);

    // HighGUI Library capture device
    CvCapture    * m_VideoCapture;

    // current Video image
    IplImage     * m_CurrentImage;
    unsigned char* m_CurrentVideoTexture;

    IplImage     * m_PauseImage;

    ///
    /// saves the video file name (is empty if a grabbing device is used or if this is not initialized)
    std::string m_VideoFileName;

    ///
    /// saves the grabbing device number (is -1 if a videofilename is used or if this is not initialized)
    short m_GrabbingDeviceNumber;

    // Repeat a video file
    bool m_RepeatVideo;

    // Switch between CVCAM Lib and HighGui Lib
    bool m_UseCVCAMLib;

    // On-the-fly undistortion of the captured video image
    bool m_UndistortImage;
    mitk::UndistortCameraImage::Pointer m_UndistortCameraImage;

    /**
    * Flag to enable or disable video flipping by X Axis.
    **/
    bool m_FlipXAxisEnabled;

    /**
    * Flag to enable or disable video flipping by Y Axis.
    **/
    bool m_FlipYAxisEnabled;
  };
}
#endif // Header


