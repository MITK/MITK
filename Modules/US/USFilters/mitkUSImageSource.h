/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef MITKUSImageSource_H_HEADER_INCLUDED_
#define MITKUSImageSource_H_HEADER_INCLUDED_

// ITK
#include <itkProcessObject.h>
#include <itkFastMutexLock.h>

// MITK
#include <MitkUSExports.h>
#include <mitkCommon.h>
#include "mitkBasicCombinationOpenCVImageFilter.h"
#include "mitkOpenCVToMitkImageFilter.h"
#include "mitkImageToOpenCVImageFilter.h"

namespace mitk {
  /**
  * \brief This is an abstract superclass for delivering USImages.
  * Each subclass must implement the method mitk::USImageSource::GetNextRawImage().
  * The public method mitk::USImageSource::GetNextImage() can the be used to
  * get the next image from the image source. This image will be filtered by
  * the filter set with mitk::USImageSource::SetImageFilter().
  *
  * \ingroup US
  */
  class MITKUS_EXPORT USImageSource : public itk::Object
  {
  public:
    static const char* IMAGE_PROPERTY_IDENTIFIER;

    mitkClassMacroItkParent(USImageSource, itk::Object);

    itkGetMacro(ImageFilter, mitk::BasicCombinationOpenCVImageFilter::Pointer);

    void PushFilter(AbstractOpenCVImageFilter::Pointer filter);
    bool RemoveFilter(AbstractOpenCVImageFilter::Pointer filter);
    bool GetIsFilterInThePipeline(AbstractOpenCVImageFilter::Pointer filter);

    /**
    * \brief Retrieves the next frame. This will typically be the next frame
    * in a file or the last cached file in a device. The image is filtered if
    * a filter was set by mitk::USImageSource::SetImageFilter().
    *
    * \return pointer to the next USImage (filtered if set)
    */
    std::vector<mitk::Image::Pointer> GetNextImage();

  protected:
    USImageSource();
    ~USImageSource() override;
    /**
    * \brief Set the given OpenCV image matrix to the next image received
    * from the device or file.
    *
    * The standard implementation calls the overloaded function with an
    * mitk::Image and converts this image to OpenCV then. One should reimplement
    * this method for a better performance if an image filter is set.
    */
    virtual void GetNextRawImage(std::vector<cv::Mat>&);

    /**
    * \brief Set mitk::Image to the next image received from the device or file.
    * This method must be implemented in every subclass.
    */
    virtual void GetNextRawImage(std::vector<mitk::Image::Pointer>&) = 0;

    /**
    * \brief Used to convert from OpenCV Images to MITK Images.
    */
    mitk::OpenCVToMitkImageFilter::Pointer m_OpenCVToMitkFilter;
    /**
    * \brief Used to convert from MITK Images to OpenCV Images.
    */
    mitk::ImageToOpenCVImageFilter::Pointer m_MitkToOpenCVFilter;

  private:
    /**
* \brief Filter is executed during mitk::USImageVideoSource::GetNextImage().
*/
    BasicCombinationOpenCVImageFilter::Pointer m_ImageFilter;

    int                                        m_CurrentImageId;

    itk::FastMutexLock::Pointer m_ImageFilterMutex;
  };
} // namespace mitk
#endif /* MITKUSImageSource_H_HEADER_INCLUDED_ */
