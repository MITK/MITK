/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef MITKUSImageVideoSource_H_HEADER_INCLUDED_
#define MITKUSImageVideoSource_H_HEADER_INCLUDED_

// ITK
#include <itkProcessObject.h>

// MITK
#include "mitkUSImageSource.h"
#include "mitkConvertGrayscaleOpenCVImageFilter.h"
#include "mitkCropOpenCVImageFilter.h"
#include "mitkBasicCombinationOpenCVImageFilter.h"

// OpenCV
#include <opencv2/videoio.hpp>

namespace mitk {
  /**
    * \brief This class can be pointed to a video file or a videodevice and delivers USImages.
    *
    * Images are in color by default, but can be set to greyscale via SetColorOutput(false),
    * which significantly improves performance.
    *
    * Images can also be cropped to a region of interest, further increasing performance.
    *
    * \ingroup US
    */
  class MITKUS_EXPORT USImageVideoSource : public mitk::USImageSource
  {
  public:
    mitkClassMacroItkParent(USImageVideoSource, itk::ProcessObject);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    /**
      * \brief Defines a region of interest by top left and bottom right corner.
      */
    struct USImageRoi
    {
      int topLeftX;
      int topLeftY;
      int bottomRightX;
      int bottomRightY;

      USImageRoi()
        : topLeftX(0), topLeftY(0), bottomRightX(0), bottomRightY(0) { };
      USImageRoi(unsigned int topLeftX, unsigned int topLeftY, unsigned int bottomRightX, unsigned int bottomRightY)
        : topLeftX(topLeftX), topLeftY(topLeftY), bottomRightX(bottomRightX), bottomRightY(bottomRightY) { };
    };

    /**
      * \brief Defines a region of interest by distances to the four image borders.
      */
    struct USImageCropping
    {
      unsigned int top;
      unsigned int bottom;
      unsigned int left;
      unsigned int right;

      USImageCropping()
        : top(0), bottom(0), left(0), right(0) { };
      USImageCropping(unsigned int top, unsigned int bottom, unsigned int left, unsigned int right)
        : top(top), bottom(bottom), left(left), right(right) { };
    };

    /**
      * \brief Opens a video file for streaming. If nothing goes wrong, the
      * VideoSource is ready to deliver images after calling this function.
      */
    void SetVideoFileInput(std::string path);

    /**
      * \brief Opens a video device for streaming. Takes the Device id. Try -1 for "grab the first you can get"
      * which works quite well if only one device is available. If nothing goes wrong, the
      * VideoSource is ready to deliver images after calling this function.
      */
    void SetCameraInput(int deviceID);

    void ReleaseInput();

    /**
      * \brief Sets the output image to rgb or grayscale.
      * Output is color by default
      * and can be set to color by passing true, or to grayscale again by passing false.
    */
    void SetColorOutput(bool isColor);

    /**
      * \brief Defines the cropping area.
      * The rectangle will be justified to the image borders if the given
      * rectangle is larger than the video source. If a correct rectangle is
      * given, the dimensions of the output image will be equal to those of the
      * rectangle.
      */
    void SetRegionOfInterest(int topLeftX, int topLeftY, int bottomRightX, int bottomRightY);

    /**
      * \brief Defines the cropping area.
      * The rectangle will be justified to the image borders if the given
      * rectangle is larger than the video source. If a correct rectangle is
      * given, the dimensions of the output image will be equal to those of the
      * rectangle.
      *
      * \param regionOfInterest struct defining x and y coordinates of top left and bottom right corner
      */
    void SetRegionOfInterest(USImageRoi regionOfInterest);

    /**
      * \brief Defines the cropping area.
      * The rectangle will be justified to the image borders if the given
      * rectangle is larger than the video source. If a correct rectangle is
      * given, the dimensions of the output image will be equal to those of the
      * rectangle.
      *
      * \param cropping struct defining distances to the four image borders
      */
    void SetCropping(USImageCropping cropping);

    /**
      * /brief Removes the region of interest.
      * Produced images will be uncropped after call of this method.
      */
    void RemoveRegionOfInterest();

    /**
      * \brief This is a workaround for a problem that happens with some video device drivers.
      *
      * If you encounter OpenCV Warnings that buffer sizes do not match while calling getNextFrame,
      * then do the following: Using the drivers control panel to force a certain resolution, then call
      * this method with the same Dimensions after opening the device.
      * Before retrieving images one should call mitk::USImageVideoSource::isReady().
      */
    void OverrideResolution(int width, int height);

    // Getter & Setter
    itkGetMacro(IsVideoReady, bool);
    itkGetMacro(ResolutionOverride, bool);
    itkSetMacro(ResolutionOverride, bool);
    itkGetMacro(IsGreyscale,bool);
    itkGetMacro(ResolutionOverrideWidth,int);
    itkGetMacro(ResolutionOverrideHeight,int);
    int GetImageHeight();
    int GetImageWidth();
    USImageCropping GetCropping();
    USImageRoi GetRegionOfInterest();

    /**
      * \brief Returns true if images can be delivered.
      *
      * Only if true is returned one can retrieve images via
      * mitk::USImageVideoSource::GetNextImage().
      * If false is returned, behaviour is undefined.
      */
    bool GetIsReady();

  protected:
    USImageVideoSource();
    ~USImageVideoSource() override;

    /**
      * \brief Next image is gathered from the image source.
      *
      * \param[out] image an OpenCV-Matrix containing this image
      */
    void GetNextRawImage( std::vector<cv::Mat>& image ) override;

    /**
      * \brief Next image is gathered from the image source.
      *
      * \param[out] image an mitk::Image containing this image
      */
    void GetNextRawImage( std::vector<mitk::Image::Pointer>& image ) override;

    /**
      * \brief The source of the video, managed internally
      */
    cv::VideoCapture* m_VideoCapture;

    /**
      * \brief If true, a frame can be grabbed anytime.
      */
    bool m_IsVideoReady;

    /**
      * \brief If true, image output will be greyscale.
      */
    bool m_IsGreyscale;

    /**
      * \brief If true, image will be cropped according to settings of crop filter.
      */
    bool m_IsCropped;

    /**
      * These Variables determined whether Resolution Override is on, what dimensions to use.
      */
    int  m_ResolutionOverrideWidth;
    int  m_ResolutionOverrideHeight;
    bool m_ResolutionOverride;

    ConvertGrayscaleOpenCVImageFilter::Pointer  m_GrayscaleFilter;
    CropOpenCVImageFilter::Pointer              m_CropFilter;
  };
} // namespace mitk
#endif /* MITKUSImageVideoSource_H_HEADER_INCLUDED_ */
